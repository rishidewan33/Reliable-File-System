#ifndef __COMMON_H__
#define __COMMON_H__

/*
 * NOTE: 
 * This file represents the constants and basic data struct to your 
 * file system.
 */

const int SECTORS_FOR_METADATA = 5;
const int NUM_OF_SECTORS = 16384;
const int MAX_SECTORS_PER_OP = 1;
const int SECTOR_SIZE = 512;
const int ADISK_REDO_LOG_SECTORS = 1024;
const int ADISK_SECTOR_OFFSET = ADISK_REDO_LOG_SECTORS + 1;
const int ADISK_HEADER_NUMBER = 6156879;
const int ADISK_COMMIT_NUMBER = 5321842;
const int TREE_METADATA_SIZE = 32;

const int MAX_TREES = 512;

const int MAX_CONCURRENT_TRANSACTIONS = 8;
const int MAX_WRITES_PER_TRANSACTION = 32;

const int FS_MAX_NAME = 32;
const int MAX_PATH = 128;

const int MAX_FD = 32;

const int READ = 0;
const int WRITE = 1;

typedef enum {SUCCESS, FAILURE} OpResult;

/*
 * For project 4. Ignore for project 3.
 */

typedef enum { PTREE_FREE_SPACE, PTREE_FREE_TREES, PTREE_MAX_TREES } TParams;

typedef enum {MAX_FILE, FREE_SPACE, MAX_FILE_SIZE, FREE_FILES} FSParams;

typedef unsigned long long TransID;

#endif
