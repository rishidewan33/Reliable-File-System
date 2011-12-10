#ifndef __DISK_H__
#define __DISK_H__


#include "common.h"
#include "sthread.h"
#include <cstdio>

/*
 * Note: all the constants are defined in common.h
 */
class Disk
{
  public:
    Disk();
    ~Disk();
    OpResult readDisk(int sectorNum, char * dataPtr);
    OpResult writeDisk(int sectorNum, char * dataPtr);
    void setCrashProb(float prob);
  private:
    OpResult diskInit();
    OpResult doDiskOp(int secnum, char* data, int numsecs, int op);
    
};
#endif
