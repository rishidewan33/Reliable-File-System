#include "ADisk.h"
#include <algorithm>
#include <cstring>
#include <pthread.h>

static char all_zeros[SECTOR_SIZE] = {0};

TransID Transaction::TransIDCounter = 0;

ADisk::ADisk(Disk disk, bool format)
{
    maindisk = disk;
    smutex_init(&adisk_lock);
    scond_init(&ok_to_create_transaction);
    scond_init(&ok_to_write_to_log);
    scond_init(&ok_to_writeback_to_disk);
    scond_init(&ok_to_clear_log);
    if (format)
    { //Fill the entire disk with zero.
        for (int i = 0; i < NUM_OF_SECTORS; ++i)
            maindisk.writeDisk(i, all_zeros);
        num_sectors = NUM_OF_SECTORS - ADISK_SECTOR_OFFSET;
        checkpoint = 0;
    }
    else
    { //Scan the log for any successfully committed transactions
        char checkpoint_sector[SECTOR_SIZE] = {0}; //First, we read the checkpoint
        maindisk.readDisk(ADISK_REDO_LOG_SECTORS, checkpoint_sector);
        checkpoint = byteArrayToInt(checkpoint_sector);
        int log_sectors_visited = 0;
        char read_sector[SECTOR_SIZE];
        while (log_sectors_visited != ADISK_REDO_LOG_SECTORS) //next, we scan the log
        {
            int start = checkpoint;

            //Read the Transaction Header Number
            maindisk.readDisk(checkpoint, read_sector);
            checkpoint = (checkpoint + 1) % ADISK_REDO_LOG_SECTORS;
            ++log_sectors_visited;

            //Check to see if the "Transaction" in the log is actually a garbage transaction
            //By verifying the sector contains the header number
            if (byteArrayToInt(read_sector) != ADISK_HEADER_NUMBER)
                continue;

            //Read transID sector
            maindisk.readDisk(checkpoint, read_sector);
            TransID id = byteArrayToTransID(read_sector);
            checkpoint = (checkpoint + 1) % ADISK_REDO_LOG_SECTORS;
            ++log_sectors_visited;

            //Read sector_locations sector
            maindisk.readDisk(checkpoint, read_sector);
            std::vector<int> sector_numbers;
            this->readSectorLocationsFromSector(read_sector, sector_numbers);
            checkpoint = (checkpoint + 1) % ADISK_REDO_LOG_SECTORS;
            ++log_sectors_visited;

            if (!sector_numbers.empty())
            {
                std::vector<char*> sector_datas;
                for (unsigned int i = 0; i < sector_numbers.size(); ++i)
                {
                    char* sector_data = new char[SECTOR_SIZE]();
                    maindisk.readDisk(checkpoint, sector_data);
                    sector_datas.push_back(sector_data);
                    checkpoint = (checkpoint + 1) % ADISK_REDO_LOG_SECTORS;
                    ++log_sectors_visited;
                }

                maindisk.readDisk(checkpoint, read_sector);
                if (byteArrayToInt(read_sector) != ADISK_COMMIT_NUMBER)
                {
                    checkpoint = (checkpoint + 1) % ADISK_REDO_LOG_SECTORS;
                    ++log_sectors_visited;
                }
                else
                {
                    checkpoint = (checkpoint + 1) % ADISK_REDO_LOG_SECTORS;
                    ++log_sectors_visited;
                    maindisk.readDisk(checkpoint, read_sector);
                    if (byteArrayToTransID(read_sector) == id) //We Serialized the Transaction successfully
                    {
                        if (sector_numbers.size() == sector_datas.size())
                        {
                            for (unsigned int i = 0; i < sector_datas.size(); ++i)
                                maindisk.writeDisk(sector_numbers.at(i), sector_datas.at(i));
                        }
                    }
                    else
                    {
                        checkpoint = (checkpoint + 1) % ADISK_REDO_LOG_SECTORS;
                        ++log_sectors_visited;
                    }
                }
                for (std::vector<char*>::iterator it = sector_datas.begin(); it != sector_datas.end(); ++it)
                    delete (*it);
            }
            for (; start != checkpoint; start = (start + 1) % ADISK_REDO_LOG_SECTORS) //Clear the log with the scanned transaction;
                maindisk.writeDisk(start, all_zeros);
        }
    }
    log_sectors = ADISK_REDO_LOG_SECTORS;
    log_head = 0;
    log_tail = 0;
    sthread_create(&writebackThread, ADisk::writebackThreadRun, (void*) this);
    sthread_create(&logThread, ADisk::logThreadRun, (void*) this);
}

ADisk::~ADisk()
{
    for (std::map<TransID, Transaction*>::iterator it = active_transaction_list.begin(); it != active_transaction_list.end(); ++it)
        delete it->second;
    for (std::deque<Transaction*>::iterator it = writeback_queue.begin(); it != writeback_queue.end(); ++it)
        delete (*it);
    active_transaction_list.clear();
    writeback_queue.clear();
    //smutex_destroy(&adisk_lock);
    //scond_destroy(&ok_to_create_transaction);
    //scond_destroy(&ok_to_log);
    //scond_destroy(&ok_to_writeback);
}

OpResult ADisk::abortTransaction(TransID xid)
{
    smutex_lock(&adisk_lock);
    OpResult result;
    if (active_transaction_list.find(xid) != active_transaction_list.end())
    {
        delete active_transaction_list[xid];
        active_transaction_list.erase(xid);
        result = SUCCESS;
    }
    else
        result = FAILURE;
    smutex_unlock(&adisk_lock);
    return result;
}

TransID ADisk::beginTransaction()
{
    smutex_lock(&adisk_lock);
    while (active_transaction_list.size() == (unsigned int) MAX_CONCURRENT_TRANSACTIONS)
    {
        scond_wait(&ok_to_create_transaction, &adisk_lock);
    }
    Transaction* new_transaction = new Transaction();
    active_transaction_list.insert(std::pair<TransID, Transaction* > (new_transaction->getTransID(), new_transaction));
    smutex_unlock(&adisk_lock);
    return new_transaction->getTransID();
}

void ADisk::readSectorLocationsFromSector(char* sector, std::vector<int>& locations)
{
    int location = -1;
    char* traverse_sector = sector;
    while ((location = byteArrayToInt(traverse_sector)))
    {
        locations.push_back(location);
        traverse_sector += sizeof (int);
    }
}

OpResult ADisk::commitTransaction(TransID xid)
{
    smutex_lock(&adisk_lock);
    if (active_transaction_list.find(xid) == active_transaction_list.end())
    {
        smutex_unlock(&adisk_lock);
        return FAILURE;
    }
    else
    {
        Transaction* t = active_transaction_list[xid];
        while (log_sectors < t->getSectorsRequired()) //Wait for required number of log sectors
        {
            scond_wait(&ok_to_write_to_log, &adisk_lock);
        }
        log_sectors -= t->getSectorsRequired();
        log_tracker.insert(std::pair<Transaction*,int>(t,log_head)); //Indicates the log location of a serialized transaction.
        log_head = (log_head + t->getSectorsRequired()) % ADISK_REDO_LOG_SECTORS; //Reserve sectors for the log;
        if (serializeTransactionToLog(t) == FAILURE)
        {
            log_tracker.erase(t);
            smutex_unlock(&adisk_lock);
            return FAILURE;
        }
        writeback_queue.push_back(t);
        active_transaction_list.erase(xid);
        scond_broadcast(&ok_to_create_transaction, &adisk_lock);
        scond_signal(&ok_to_writeback_to_disk, &adisk_lock);
    }
    smutex_unlock(&adisk_lock);
    return SUCCESS;
}

OpResult ADisk::serializeTransactionToLog(Transaction* t)
{
    if (!t)
        return FAILURE;
    
    char* serialized_transaction = new char[SECTOR_SIZE * t->getSectorsRequired()]();
    char* serialize_ptr = serialized_transaction;

    char* serialized_header_num = intToByteArray(ADISK_HEADER_NUMBER);
    memcpy(serialize_ptr, serialized_header_num, sizeof (serialized_header_num));
    serialize_ptr += SECTOR_SIZE;

    char* serialized_transid = transIDToByteArray(t->getTransID());
    memcpy(serialize_ptr, serialized_transid, sizeof (serialized_transid));
    serialize_ptr += SECTOR_SIZE;

    std::vector<int> sector_num_bytes = t->getSectorNumbers();
    char* sector_num_traverse = serialize_ptr;
    for (std::vector<int>::iterator it = sector_num_bytes.begin(); it != sector_num_bytes.end(); ++it)
    {
        char* write_sector_num = intToByteArray((*it));
        memcpy(sector_num_traverse, write_sector_num, sizeof (write_sector_num));
        sector_num_traverse += sizeof (write_sector_num);
        delete write_sector_num;
    }
    serialize_ptr += SECTOR_SIZE;

    char* sector_data_bytes;
    for (std::vector<int>::iterator it = sector_num_bytes.begin(); it != sector_num_bytes.end(); ++it)
    {
        sector_data_bytes = t->getDataAtSectorNum((*it));
        char buffer[SECTOR_SIZE];
        for (int i = 0; i < SECTOR_SIZE; ++i)
            buffer[i] = sector_data_bytes[i];
        memcpy(serialize_ptr, buffer, SECTOR_SIZE);
        serialize_ptr += SECTOR_SIZE;
    }

    char* serialized_commit_number = intToByteArray(ADISK_COMMIT_NUMBER);
    memcpy(serialize_ptr, serialized_commit_number, sizeof (serialized_commit_number));
    serialize_ptr += SECTOR_SIZE;

    memcpy(serialize_ptr, serialized_transid, sizeof (serialized_transid));
    serialize_ptr += SECTOR_SIZE;

    OpResult result;
    if (serialize_ptr - serialized_transaction == (SECTOR_SIZE * t->getSectorsRequired()))
    {
        char buf[SECTOR_SIZE];
        int i = 0;
        int log_transaction_start = log_tracker[t];
        int log_transaction_end = (log_transaction_start + t->getSectorsRequired()) % ADISK_REDO_LOG_SECTORS;
        while (log_transaction_start != log_transaction_end)
        {
            for (int j = 0; j < SECTOR_SIZE; ++j)
                buf[j] = serialized_transaction[SECTOR_SIZE * i + j];
            maindisk.writeDisk(log_transaction_start, buf);
            ++i;
            log_transaction_start = (log_transaction_start + 1) % ADISK_REDO_LOG_SECTORS;
        }
        result = SUCCESS;
    }
    else
    {
        result = FAILURE;
    }
    //Free the resources allocated for the serialized transaction.
    delete serialized_transaction;
    delete serialized_header_num;
    delete serialized_transid;
    delete serialized_commit_number;
    return result;
}

void ADisk::writebackTransactionsToDisk()
{
    smutex_lock(&adisk_lock);

    //Thread waits when the WriteBack list is empty.
    while (writeback_queue.empty())
    {
        scond_wait(&ok_to_writeback_to_disk, &adisk_lock);
    }

    //Grab the next transaction off the list to be written.
    Transaction* t = writeback_queue.front();
    //Write data from transaction onto disk.
    std::vector<int> traversal = t->getSectorNumbers();
    for (std::vector<int>::iterator it = traversal.begin(); it != traversal.end(); ++it)
    {
        char* buf = t->getDataAtSectorNum(*it);
        maindisk.writeDisk(*it, buf);
    }
    
    //WriteBack is now complete.
    completed_tickets.insert(std::pair<int, Transaction*> (log_tracker[t], t));
    log_tracker.erase(t);
    
    scond_broadcast(&ok_to_clear_log, &adisk_lock);
    smutex_unlock(&adisk_lock);
}

void* ADisk::writebackThreadRun(void* arg)
{
    ADisk *threaddisk = (ADisk*) arg;
    while (1)
    {
        threaddisk->writebackTransactionsToDisk();
    }
    return 0;
}

char* ADisk::transIDToByteArray(const TransID& xid)
{
    char* byteArray = new char[sizeof (TransID)]();
    for (unsigned int i = 0; i < sizeof (TransID); ++i)
        byteArray[i] = ((xid >> (8 * (sizeof (TransID) - 1 - i))) & 0xFF);
    return byteArray;
}

TransID ADisk::byteArrayToTransID(char* byteArray)
{
    TransID transid = 0;
    for (unsigned int i = 0; i < sizeof (TransID); ++i)
    {
        transid |= ((byteArray[i] & 0xFF) << (8 * (sizeof (TransID) - 1 - i)));
    }
    return transid;
}

char* ADisk::intToByteArray(const int& value)
{
    char* byteArray = new char[sizeof (int) ]();
    for (unsigned int i = 0; i < sizeof (int); ++i)
        byteArray[i] = ((value >> (8 * (sizeof (int) - 1 - i))) & 0xFF);
    return byteArray;
}

int ADisk::byteArrayToInt(char* byteArray)
{
    int integer = 0;
    for (unsigned int i = 0; i < sizeof (int); ++i)
    {
        integer |= ((byteArray[i] & 0xFF) << (8 * (sizeof (int) - 1 - i)));
    }
    return integer;
}

void ADisk::compactLogWhenNeeded()
{
    smutex_lock(&adisk_lock);

    while (log_head == log_tail || completed_tickets.find(log_tail) == completed_tickets.end())
    {
        scond_wait(&ok_to_clear_log, &adisk_lock);
    }
    
    Transaction* trans_clear = completed_tickets[log_tail];

    int sectors = trans_clear->getSectorsRequired();
    int start = log_tail;
    int end = (log_tail + sectors) % ADISK_REDO_LOG_SECTORS;
    
    while (start != end) //Clear the specific part of the log
    {
        maindisk.writeDisk(start, all_zeros);
        start = (start + 1) % ADISK_REDO_LOG_SECTORS;
    }
    delete trans_clear;
    writeback_queue.erase(std::find(writeback_queue.begin(),writeback_queue.end(),trans_clear));
    completed_tickets.erase(log_tail);
    log_tail = end;
    char* checkpoint_sector = intToByteArray(log_tail);
    maindisk.writeDisk(ADISK_REDO_LOG_SECTORS, checkpoint_sector); //Write the new tail to the checkpoint sector.
    delete checkpoint_sector;
    log_sectors += sectors; //recollect the sectors used for this former transaction.
    scond_broadcast(&ok_to_write_to_log, &adisk_lock);
    smutex_unlock(&adisk_lock);
}

void* ADisk::logThreadRun(void* arg)
{
    ADisk* threaddisk = (ADisk*) arg;
    while (1)
    {
        threaddisk->compactLogWhenNeeded();
    }
    return 0;
}

/**
   Gets the current number of non-log sectors in the current disk.

   @return Description of returned value.
 */
int ADisk::getNSectors()
{
    return num_sectors;
}

/**  
 Reads data from a sector in an active transaction with the TransID 'xid' at sector location 'sectorNum',
 and fills the dataPtr buffer with the read data. If the transaction wasn't found in the active transaction
 list or the found active transaction never wrote to that specific sector, the WriteBack List is checked.
 Otherwise, the data is then read from disk at that sector and filled into dataPtr. 

   @param     xid The transaction id to read the sector from.
   @param     sectorNum The sector location to read data from.
   @param     dataPtr The buffer to fill the sector data in.
   @return    An OpResult indicating the success or failure of the operation. FAILURE can be return of the requested sector number is out of bounds.
 */
OpResult ADisk::readSector(TransID& xid, int sectorNum, char* dataPtr)
{
    smutex_lock(&adisk_lock);
    sectorNum += ADISK_SECTOR_OFFSET;
    if (sectorNum < ADISK_SECTOR_OFFSET || sectorNum >= NUM_OF_SECTORS)
    {
        smutex_unlock(&adisk_lock);
        return FAILURE;
    }
    if (active_transaction_list.find(xid) != active_transaction_list.end())
    {
        char* read_atl = active_transaction_list[xid]->getDataAtSectorNum(sectorNum);
        if (read_atl)
        {
            for (int i = 0; i < SECTOR_SIZE; ++i)
                dataPtr[i] = read_atl[i];
            smutex_unlock(&adisk_lock);
            return SUCCESS;
        }
    }
    char* read_wbl;
    for (std::deque<Transaction*>::iterator it = writeback_queue.begin(); it != writeback_queue.end(); ++it)
    {
        if ((read_wbl = (*it)->getDataAtSectorNum(sectorNum)))
        {
            for (int i = 0; i < SECTOR_SIZE; ++i)
                dataPtr[i] = read_wbl[i];
            smutex_unlock(&adisk_lock);
            return SUCCESS;
        }
    }
    maindisk.readDisk(sectorNum, dataPtr);
    smutex_unlock(&adisk_lock);
    return SUCCESS;
}

/**
 Add sector data to be written at sector location sectorNum to an active transaction.
  
   @param     xid The TransID of the active transaction to write the sector to.
   @param     sectorNum The sector location to write data to on disk.
   @param     dataPtr The buffer containing data the sector on disk is to be written to.   
   @return    An OpResult indicating the success or failure of the operation. FAILURE can be returned if 'sectorNum' is out of bounds, or if 'xid' doesn't refer to an active transaction.
 */
OpResult ADisk::writeSector(TransID& xid, int sectorNum, char* dataPtr)
{
    /*
     * Pre-requisite for dataPtr: It assumes that dataPtr is allocated to the heap.
     * The memory for dataPtr is eventually free'd in the Transaction destructor.
     */
    smutex_lock(&adisk_lock);
    sectorNum += ADISK_SECTOR_OFFSET;
    if (sectorNum < ADISK_SECTOR_OFFSET || sectorNum >= NUM_OF_SECTORS)
    {
        smutex_unlock(&adisk_lock);
        return FAILURE;
    }
    OpResult result = SUCCESS;
    if (active_transaction_list.find(xid) == active_transaction_list.end())
        result = FAILURE;
    else
    {
        if (active_transaction_list[xid]->getTotalWrites() == (unsigned int) MAX_WRITES_PER_TRANSACTION)
            result = FAILURE;
        else
            active_transaction_list[xid]->addSector(sectorNum, dataPtr);
    }
    smutex_unlock(&adisk_lock);
    return result;
}

std::vector<TransID> ADisk::getActiveTransactionIDs()
{
    std::vector<TransID> active_ids;
    for (std::map<TransID, Transaction*>::iterator it = active_transaction_list.begin(); it != active_transaction_list.end(); ++it)
        active_ids.push_back((*it).first);
    return active_ids;
}