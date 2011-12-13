#include <cassert>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include "ADiskTests.h"

ADiskUnit::ADiskUnit(ADisk adisk) : adisk(adisk)
{
    smutex_init(&lock);
    srand(time(0));
}

ADiskUnit::~ADiskUnit()
{
    smutex_destroy(&lock);
}

void ADiskUnit::testSizeOfDiskLists()
{
    smutex_lock(&lock);
    assert(adisk.active_transaction_list.empty());
    assert(adisk.writeback_queue.empty());
    std::cout << "testSizeOfDiskLists() PASSED\n";
    smutex_unlock(&lock);
}

bool ADiskUnit::bufferEquals(char buf1[], char buf2[])
{
    for (int i = 0; i < SECTOR_SIZE; ++i)
    {
        if (buf1[i] != buf2[i])
            return false;
    }
    return true;
}

void ADiskUnit::randomBufferFill(char* buffer)
{
    for (int i = 0; i < SECTOR_SIZE; ++i)
        buffer[i] = rand() % 256;
}

void ADiskUnit::testTransactionFunctionality()
{
    smutex_lock(&lock);
    //Test beginTransaction()
    assert(adisk.active_transaction_list.empty());
    TransID transID = adisk.beginTransaction();
    assert(adisk.active_transaction_list.size() == 1);

    //test that acquired Transaction is the same as the one
    //on the ATL
    Transaction* testTrans = adisk.active_transaction_list[transID];
    //Test that Transaction created correctly
    assert(testTrans->getTransID() == transID);
    assert(testTrans->getSectorNumbers().empty());
    assert(testTrans->getTotalWrites() == 0);

    //test writeSector()
    char* writeTwos = new char[SECTOR_SIZE]();
    for(int i = 0 ; i < SECTOR_SIZE; ++i)
        writeTwos[i] = 2;
    int someSectorNum = 5500;
    adisk.writeSector(transID, someSectorNum, writeTwos);
    assert(testTrans->getSectorNumbers().size() == 1);
    assert(testTrans->getTotalWrites() == 1);
    assert(bufferEquals(testTrans->getDataAtSectorNum(someSectorNum+ADISK_SECTOR_OFFSET), writeTwos));
    assert(testTrans->getSectorNumbers().at(0) == someSectorNum+ADISK_SECTOR_OFFSET);
    for (int i = 0; i < 15; ++i)
    {
        char* testBuff = new char[SECTOR_SIZE]();
        randomBufferFill(testBuff);
        adisk.writeSector(transID, 3000 + i, testBuff);
        assert(bufferEquals(testTrans->getDataAtSectorNum(3000 + i + ADISK_SECTOR_OFFSET), testBuff));
        assert(testTrans->sectors.find(i+3000+ADISK_SECTOR_OFFSET) != testTrans->sectors.end());
    }

    assert(testTrans->getSectorNumbers().size() == 16);
    assert(testTrans->getTotalWrites() == 16);

    //test abortTransaction(tid)
    assert(adisk.active_transaction_list.size() == 1);
    adisk.abortTransaction(transID);
    assert(adisk.active_transaction_list.size() == 0);
    assert(adisk.abortTransaction(transID) == FAILURE);
    std::cout << "testTransactionFunctionality() PASSED\n";
    smutex_unlock(&lock);
}

void ADiskUnit::testTransactionClass()
{
    smutex_lock(&lock);

    //test beginTransaction() again
    for(int i = 0; i < 5; ++i)
        adisk.beginTransaction();
    assert(adisk.active_transaction_list.size() == 5);
    Transaction* transByteArray = adisk.active_transaction_list[adisk.beginTransaction()];
    assert(adisk.active_transaction_list.size() == 6);
    
    //transByteArray->addSector(1056, ALL_ONES);
    //transByteArray->addSector(1057, ALL_TWOS);
    //transByteArray->addSector(1058, ALL_THREES);

    //testing that our loop in Transaction.addWrite(...)
    //correctly updates the sector as the most recent
    for (int ii = 0; ii < 60; ii++)
    {
        //transByteArray->addSector(1058, ALL_THREES);
    }

    adisk.commitTransaction(transByteArray->getTransID());

    //assert(toAndFromByteArrayWorks(transByteArray));
    //assert(testParseLogBytes(transByteArray));
    
    // Test Transaction functionality
    Transaction* transTest = new Transaction();
    transTest->id = 99;
    testTransaction(transTest);

    //Test WriteBackList
    testWriteBackList();
    std::cout << "testTransactionClass() PASSED\n";
    smutex_unlock(&lock);
}

void ADiskUnit::testTransaction(Transaction* t)
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::testCommitTransactions()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::cs372Midterm2Tests()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::testWriteBackList()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::testMDToByteArray()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::testLogManagement()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::testTransCommits()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::smallRecoveryTest()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::testRecovery()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}

void ADiskUnit::megaTest()
{
    smutex_lock(&lock);
    smutex_unlock(&lock);
}
