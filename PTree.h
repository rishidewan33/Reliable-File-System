#ifndef __PTree_H__
#define __PTree_H__


#include "ADisk.h"

class PTree
{
  public:
  /*
   * You should not need to change the public interface
   */
    PTree(ADisk adisk, int doFormat); 
    ~PTree();
    TransID beginTrans();
    int commitTrans(TransID xid);
    int abortTrans(TransID xid);
    int createTree(TransID xid);
    int deleteTree(TransID xid, int tnum);
    int getMaxDataBlockId(TransID xid, int tnum); 
    int readData(TransID xid, int tnum, int blockId, char *buffer);
    int writeData(TransID xid, int tnum, int blockId, char * buffer);
    int readTreeMetadata(TransID xid, int tnum, char *buffer);
    int writeTreeMetadata(TransID xid, int tnum, char *buffer);
    int getParam(TParams param);

  private:
  /*
   * You will need to add more member variables here.
   */
};
#endif
