#ifndef __FlatFS_H__
#define __FlatFS_H__


#include "PTree.h"


class FlatFS
{
  public:
  /*
   * You should not need to change the public interface
   */
    FlatFS(PTree perstTrees, int doFormat); 
    ~FlatFS();
    TransID beginTrans();
    int commitTrans(TransID xid);
    int abortTrans(TransID xid);
    int ICreateFile(TransID xid);
    int IDeleteFile(TransID xid, int inumber);
    int IRead(TransID xid, int inumber, int count, int offset, char *buffer);
    int IWrite(TransID xid, int inumber, int count, int offset, char * buffer);
    int IGetParam(FSParams param);
    
  private:
  /*
   * You will need to add more member variables here.
   */

};
#endif
