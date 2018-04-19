/***********************************************************************
 * fstypes.h
 * Tom Kelliher, Goucher College (c) 2016
 *
 * Definitions for file system constants and data types.
 ***********************************************************************/


#ifndef __FSTYPES_H
#define __FSTYPES_H


#include <stdint.h>
#include "driver.h"


/* The number of blocks (sectors) in a floppy file system's FAT.  Note
 * that this number can be read from a field in the boot block.
 */
#define FAT_BLOCKS 9


/* The physical block number of the first block of the first and second 
 * FATs.
 */
#define FAT1_START 1
#define FAT2_START 10


/* The number of blocks (sectors) in a floppy file system's root
 * directory.  Note that this number can be calculated from information
 * specified in the boot block.
 */
#define ROOT_BLOCKS 14


/* The physical block number of the first block of the root directory. */
#define ROOT_START 19


/* Boot block entries relevant to the file system.  This is not needed
 * for the project; it's for reference purposes.  Note the use of the
 * packed attribute to keep the compiler from word-aligning the
 * structure's members.
 */
typedef struct __attribute__ ((__packed__)) bootblock_t
{
   uint8_t ignore1[11];
   uint16_t bytesPerSector;
   uint8_t sectorsPerCluster;
   uint16_t numReservedSectors;
   uint8_t numFATs;
   uint16_t maxNumRootDirEntries;
   uint16_t totalSectors;
   uint8_t ignore2;
   uint16_t sectorsPerFAT;
   uint16_t sectorsPerTrack;
   uint16_t numHeads;
   uint8_t ignore3[4];
   uint32_t totalSectorCountFAT32;
   uint8_t ignore4[2];
   uint8_t bootSignature;
   uint32_t volumeId;
   unsigned char volumeLabel[11];
   unsigned char filesystemType [8];
   uint8_t ignore5[450];
} bootblock_t;


/* Data type to use for caching the FAT in memory while the file system
 * is mounted.  The cached FAT should be flushed to disk before
 * unmounting the file system.  Note that this is an array type.
 */
typedef uint8_t fat_t[FAT_BLOCKS * BLOCKSIZE];


/* Data type to use for caching the root directory in memory while
 * the file system is mounted.  The cached root directory should be
 * flushed to disk before unmounting the file system.  Note that this is
 * an array type.
 */
typedef uint8_t root_t[ROOT_BLOCKS * BLOCKSIZE];


/* Number of directory entries per block. */
#define DIR_ENTRIES 16


/* Directory entry structure.  Again, note the use of the packed
 * attribute.
 *
 * A certain amount of chicanery is neccessary to extract a directory
 * entry from the root directory, or from a block holding a portion of a
 * sub-directory:
 *
 *    root_t root;
 *    block_t block;
 *    // Access directory entries in root via dirArray as array or pointer
 *    direntry_t *dirArray = (direntry_t *) root;
 *    dirArray[0].filename[0] ...  // Access as array
 *    dirArray->filename[0] ...   // Access as pointer
 *    // Similar for directory entries in a block:
 *    dirArray = (direntry_t *) block;
 *    // ...
 */
typedef struct __attribute__ ((__packed__)) direntry_t
{
   unsigned char filename[8];
   unsigned char extension[3];
   uint8_t attributes;
   uint16_t reserved;
   uint16_t creationTime;
   uint16_t creationDate;
   uint16_t lastAccess;
   uint16_t ignore;
   uint16_t lastWriteTime;
   uint16_t lastWriteDate;
   uint16_t firstSector;
   uint32_t fileSize;
} direntry_t;


/* File attribute masks */
#define READ_ONLY 0x1
#define HIDDEN 0x2
#define SYSTEM 0x4
#define VOLUME_LABEL 0x8
#define SUBDIRECTORY 0x10
#define ARCHIVE 0x20


#endif
