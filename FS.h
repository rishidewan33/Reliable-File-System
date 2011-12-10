#ifndef __FS_H__
#define __FS_H__


#include "FlatFS.h"


class FS
{
  public:
  /*
   * You should not need to change the public interface
   */
    FS(FlatFS *flatFS, int doFormat); 
    ~FS();
    int open(char * name);
    int close(int fd);
    int createFile(char * filename);
    int createDir(char * dirname);
    int unlink(char *name);
    int rename(char *name1, char *name2);
    int read(int fd, int count, int offset, char * buffer);
    int write(int fd, int count, int offset, char * buffer);
    int readDir(char * dirname, int bytes, char * names);
    int size(int fd);
    int sizeDir(char *dirname);
    
  private:
  /*
   * You will need to add more member variables here.
   */

};
#endif
