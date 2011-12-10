#ifndef _DIRENT_H_
#define _DIRENT_H_
/*
 * Template for a directory entry 
 * You are welcome to change this as needed
 */
#include "common.h"
struct direct
{
	unsigned flags;
	unsigned len;
	int inumber;
	char name[FS_MAX_NAME];     /* Note: a fixed-size array is
				     * simple, but inefficient. An
				     * alternative is to use a variable
				     * size array here
				     */
};
#endif _DIRENT_H_
