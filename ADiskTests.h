/* 
 * File:   ADiskTests.h
 * Author: rishi
 *
 * Created on December 11, 2011, 11:32 PM
 */

#ifndef ADISKTESTS_H
#define	ADISKTESTS_H

#include "ADisk.h"

class ADiskUnit
{
public:
    ADiskUnit(ADisk adisk);
    ~ADiskUnit();
    void testSizeOfDiskLists();
    void testTransactionFunctionality();
    void testTransactionClass();
    void testTransaction(Transaction* t);
    void testCommitTransactions();
    void cs372Midterm2Tests();
    void testWriteBackList();
    void testMDToByteArray();
    void testLogManagement();
    void testTransCommits();
    void smallRecoveryTest();
    void testRecovery();
    void megaTest();
    void randomBufferFill(char* buffer);
    bool bufferEquals(char* buf1, char* buf2);
    
private:
    Disk disk;
    ADisk adisk;
    smutex_t lock;
};

#endif	/* ADISKTESTS_H */

