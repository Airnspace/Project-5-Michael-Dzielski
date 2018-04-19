/***********************************************************************
 * driver.c
 * Tom Kelliher, Goucher College (c) 2016
 *
 * Make no changes to this file!
 *
 * See driver.h for documentation.
 ***********************************************************************/


#include "driver.h"

int fdimgopen(const char *pathname)
{
   return open(pathname, O_RDWR);
}


int fdimgclose(int device)
{
   return close(device);
}


int readblock(int device, block_t buf, unsigned int blocknum)
{
   if (lseek(device, blocknum * BLOCKSIZE, SEEK_SET) == -1)
      return -1;

   if (read(device, (void *) buf, BLOCKSIZE) == BLOCKSIZE)
      return 0;
   else
      return -1;
}


int writeblock(int device, block_t buf, unsigned int blocknum)
{
   if (lseek(device, blocknum * BLOCKSIZE, SEEK_SET) == -1)
      return -1;

   if (write(device, (void *) buf, BLOCKSIZE) == BLOCKSIZE)
      return 0;
   else return -1;
}
