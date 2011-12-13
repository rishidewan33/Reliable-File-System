#include "Disk.h"
#include "sthread.h"
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <string>

using namespace std;

static char zero_buffer[SECTOR_SIZE] = {0};
static smutex_t lock;
static int initialized = 0;
static FILE * fdisk;
static float bombProb;

Disk::Disk()
{
  if (diskInit() == FAILURE) 
  {
    cout << "Initialization of Disk failed\n";
    exit(-1);
  }
}
Disk::~Disk()
{
    fclose(fdisk);
}

OpResult Disk::diskInit()
{
  smutex_init(&lock);
  initialized = 1;
  bombProb = 0.0;
  if(!fdisk && !(fdisk = fopen("FakeDisk", "r+")))
  {
      cout << "FakeDisk not found. Creating one.\n";
      fdisk = fopen("FakeDisk","w+");
      for(int i = 0; i < NUM_OF_SECTORS; ++i)
          doDiskOp(i,zero_buffer,1,WRITE);
  }
  return SUCCESS;
}

OpResult Disk::doDiskOp(int secnum, char * data, int numsecs, int op)
{
  smutex_lock(&lock);
  assert(initialized != 0);

  unsigned bytes = numsecs * SECTOR_SIZE;
  if(secnum < 0 || (secnum + numsecs > NUM_OF_SECTORS)){
    smutex_unlock(&lock);
    return FAILURE;
  }
  if(numsecs <= 0 || numsecs > MAX_SECTORS_PER_OP){
    smutex_unlock(&lock);
    return FAILURE;
  }
  if(fseek(fdisk, secnum * SECTOR_SIZE, SEEK_SET) != 0){
    smutex_unlock(&lock);
    return FAILURE;
  }
  if(bombProb > 0.0){
    float coin = (float)(sutil_random()%1000000);
    coin = coin / (float)1000000;
    assert(coin <= 1.0000001); // Account for fp rounding errors
    assert(coin >= 0.0);
    //cout << "coin is " << coin << "\n";
    if(coin < bombProb){
      cout << "Disk Crash!!!\n";
      exit(-1);
    }
  }
  
  if(op == READ && fread(data, sizeof(char), bytes, fdisk) != bytes){
    smutex_unlock(&lock);
    return FAILURE;
  }
  if(op == WRITE && fwrite(data, sizeof(char), bytes, fdisk) != bytes){
    smutex_unlock(&lock);
    return FAILURE;
  }
  smutex_unlock(&lock);
  return SUCCESS;
}

OpResult Disk::readDisk(int secnum, char * data)
{
  return doDiskOp(secnum, data, 1, READ);
}

OpResult Disk::writeDisk(int secnum, char * data)
{
  return doDiskOp(secnum, data, 1, WRITE);
}

void Disk::setCrashProb(float prob)
{
  smutex_lock(&lock);
  assert(initialized != 0);
  bombProb = prob;
  cout << "crash probability is " << bombProb << "\n";
  smutex_unlock(&lock);
  return;
}