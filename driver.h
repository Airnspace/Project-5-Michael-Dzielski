/***********************************************************************
 * driver.h
 * Tom Kelliher, Goucher College (c) 2016
 *
 * Make no changes to this file!
 *
 * Together, driver.h and driver.c simulate driver-level floppy
 * diskette functionality using a file that contains a FAT12 floppy
 * image
 *
 * This layer passes blocks between itself and the file system layer.
 * (See the typedef for block_t below.)
 ***********************************************************************/


#ifndef __DRIVER_H
#define __DRIVER_H


#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


/* For FAT12, the block size is one sector, which is 512 bytes. */
#define BLOCKSIZE 512


/* Create a type, block_t, defined to be BLOCKSIZE unsigned 8 bit
 * integers.  Note that this is an array type.
 */
typedef uint8_t block_t[BLOCKSIZE];


/* Open the floppy diskette image in the file pathname for reading or
 * writing.
 *
 * Returns the device descriptor on success.  Returns -1 on failure.
 */
int fdimgopen(const char *pathname);


/* Close the floppy diskette with the given device descriptor.
 *
 * Returns 0 on success.  Otherwise, returns -1.
 */
int fdimgclose(int device);


/* Read a block of data from the given device into buf.  Blocknum specifies
 * which block to read.  buf must be the address of a buffer of type
 * block_t.  blocknum is the physical block number on the device.
 *
 * Returns 0 on success.  Otherwise, returns -1.
 */
int readblock(int device, block_t buf, unsigned int blocknum);


/* Write a block of data to the given device from buf.  Blocknum specifies
 * which block to write.  buf must be the address of a buffer of type
 * block_t.  blocknum is the physical block number on the device.
 *
 * Returns 0 on success.  Otherwise, returns -1.
 */
int writeblock(int device, block_t buf, unsigned int blocknum);


#endif
