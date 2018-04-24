/***********************************************************************
 * exercise2.c
 * Tom Kelliher, Goucher College (c) 2018
 *
 * "Proprietary" testing for Project 5:
 *
 *    - Ensure that we can't CD into a regular file.
 *    - Ensure that we can't TYPE a directory.
 *    - Ensure that we can't DEL a directory.
 *    - Ensure that file names are converted to uppercase.
 *    - Ensure that we can't DEL a deleted file.
 *    - Ensure that we can't TYPE a deleted file.
 *    - Ensure that we can't create a file using an in-use name.
 *    - Ensure that a file in a sub-directory can be deleted correctly.
 *    - Ensure that we can't append to a file that doesn't exist.
 ***********************************************************************/


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "fsops.h"


int main(void)
{
   int dev;
   char string[16] = "file3.txt";

   assert((dev = fd_mount("floppyData.img")) != -1);

   assert(fd_cd("CHOICE.COM") == -1);

   assert(fd_type("WINDOWS") == -1);

   assert(fd_del("WINDOWS") == -1);

   assert(fd_cd("NEW") == 0);
   assert(fd_dir(0) == 23);
   assert(fd_type(string) == 25696);
   assert(fd_del(string) == 51);
   assert(fd_del(string) == -1);
   string[0] = (char) 0xe5;
   assert(fd_del(string) == -1);
   printf("============================================================"
          "==========\n");
   printf ("Trying to TYPE a deleted file.\n");
   printf("============================================================"
          "==========\n");
   assert(fd_type(string) == -1);
   assert(fd_type("NOFILE.TXT") == -1);
   assert(fd_creat("FILEJ.TXT") == -1);
   assert(fd_del("FILEJ.TXT") == 51);
   assert(fd_dir(0) == 21);
   assert(fd_append("NOFILE.TXT", "Data", 4) == -1);

   assert(fd_unmount(dev) != -1);

   printf("\n\nRun\n"
          "   /sbin/dosfsck -v floppyData.img\n"
          "and check for errors.\n");
   printf("The last two lines of output should be:\n"
          "   Checking for unused clusters.\n"
          "   floppyData.img: 55 files, 2648/2847 clusters\n");

   return 0;
}
