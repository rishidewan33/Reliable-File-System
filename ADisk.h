#ifndef __ADISK_H__
#define __ADISK_H__

#include "Disk.h"
#include "sthread.h"
#include "common.h"
#include <map>
#include <deque>
#include <vector>

struct LogTicket
{
    TransID trans_id;
    int logSectorStart;

    LogTicket(TransID i, int l) : trans_id(i), logSectorStart(l)
    {
    }

    ~LogTicket()
    {
    }
};

class Transaction
{
    friend class ADiskUnit;
public:

    Transaction()
    {
        id = TransIDCounter++;
    }

    ~Transaction()
    {
        for (std::map<int, char*>::iterator it = sectors.begin(); it != sectors.end(); ++it) //Free all memory used for sector data.
            delete it->second;
        sectors.clear();
    }

    TransID getTransID()
    {
        return id;
    }

    unsigned int getTotalWrites()
    {
        return sectors.size();
    }

    std::vector<int> getSectorNumbers()
    {
        std::vector<int> sectorlocations;
        std::map<int, char*>::iterator it;
        for (it = sectors.begin(); it != sectors.end(); ++it)
            sectorlocations.push_back(it->first);
        return sectorlocations;
    }

    char* getDataAtSectorNum(int sectorNum)
    {
        if (sectors.find(sectorNum) == sectors.end())
            return NULL;
        return sectors[sectorNum];
    }

    OpResult addSector(int sectorNum, char* dataPtr)
    {
        if (sectors.find(sectorNum) != sectors.end())
            delete sectors[sectorNum]; //Free memory of sector that will be overwritten.
        sectors[sectorNum] = dataPtr;
        return SUCCESS;
    }

    int getSectorsRequired()
    {
        return SECTORS_FOR_METADATA + getTotalWrites();
    }
private:
    TransID id;
    std::map<int, char*> sectors;
    static unsigned long long TransIDCounter;
};

class ADisk
{
    friend class ADiskUnit;
public:
    /*
     * You should not need to change the public interface
     */
    ADisk(Disk disk, bool format);
    ~ADisk();
    int getNSectors();
    TransID beginTransaction();
    OpResult commitTransaction(TransID xid);
    OpResult abortTransaction(TransID xid);
    OpResult readSector(TransID& xid, int sectorNum, char *dataPtr);
    OpResult writeSector(TransID& xid, int sectorNum, char *dataPtr);
    std::vector<TransID> getActiveTransactionIDs();
    void writebackTransactionsToDisk();
    void compactLogWhenNeeded();
    static void* writebackThreadRun(void* arg);
    static void* logThreadRun(void* arg);

private:
    Disk maindisk;
    std::map<TransID, Transaction*> active_transaction_list;
    std::deque<Transaction*> writeback_queue;
    std::map<Transaction*, int> log_tracker;
    std::map<int, Transaction*> completed_tickets; //A mapping of the starting sector number of a written-back transaction in a log to a pointer of a Transaction.
    smutex_t adisk_lock;
    scond_t ok_to_create_transaction;
    scond_t ok_to_write_to_log;
    scond_t ok_to_writeback_to_disk;
    scond_t ok_to_clear_log;
    int num_sectors;
    int log_sectors;
    int log_head;
    int log_tail;
    int checkpoint;
    sthread_t writebackThread; //Worker thread that keeps continuously writes committed transaction data to Disk.
    sthread_t logThread; //Worker thread that continuously clears the log when needed after transactions have been successfully written to Disk.
    OpResult serializeTransactionToLog(Transaction* t);
    void readSectorLocationsFromSector(char* sector, std::vector<int>& locations);

    static char* transIDToByteArray(const TransID& xid);
    static TransID byteArrayToTransID(char* byteArray);
    static char* intToByteArray(const int& value);
    static int byteArrayToInt(char* value);
    /*
     * You will need to add more member variables here.
     */

};
#endif
