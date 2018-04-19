/***********************************************************************
 * exercise.c
 * Tom Kelliher, Goucher College (c) 2016
 *
 * Test suite for Project 5.
 *
 * Note the first comment below.  You're not finished the project until
 * you've tested all your file system operations.
 *
 * You'll need to use mdir and/or dosfsck to truly determine if you
 * implementations of fd_del(), fd_creat(), and fd_append() are working
 * correctly.
 ***********************************************************************/

/* Uncomment the following to test fd_del(), fd_creat(), and fd_append().
 * Once writes have been made to the floppy image, it must be restored
 * to its original state before rerunning this program.
 */
//#define TEST_WRITES


#include <stdio.h>
#include <assert.h>
#include "fsops.h"


int main(void)
{
   int dev;

   assert((dev = fd_mount("floppyData.img")) != -1);

   printf("============================================================"
          "==========\n");
   printf ("First root directory listing; no hidden files.\n");
   printf("============================================================"
          "==========\n");
   assert(fd_dir(0) == 27);

   printf("============================================================"
          "==========\n");
   printf ("Second root directory listing; hidden files.\n");
   printf("============================================================"
          "==========\n");
   assert(fd_dir(1) == 29);

   assert(fd_type("FILE1.TXT") == -1);
   printf("============================================================"
          "==========\n");
   printf ("Typing TROUBLE.TXT\n");
   printf("============================================================"
          "==========\n");
   assert(fd_type("TROUBLE.TXT") == 17749);

   assert(fd_cd("SUB") == -1);
   assert(fd_cd("NEW") == 0);
   printf("============================================================"
          "==========\n");
   printf ("NEW sub-directory listing\n");
   printf("============================================================"
          "==========\n");
   assert(fd_dir(0) == 23);
   printf("============================================================"
          "==========\n");
   printf ("Typing FILE1.TXT\n");
   printf("============================================================"
          "==========\n");
   assert(fd_type("FILE1.TXT") == 1538);
   printf("============================================================"
          "==========\n");
   printf ("Typing FILEK.TXT\n");
   printf("============================================================"
          "==========\n");
   assert(fd_type("FILEK.TXT") == 25696);
   assert(fd_cd("SUB") == 0);
   assert(fd_cd("SUBSUB") == 0);
   assert(fd_cd("..") == 0);
   assert(fd_cd("SUBSUB") == 0);
   printf("============================================================"
          "==========\n");
   printf ("SUBSUB sub-directory listing\n");
   printf("============================================================"
          "==========\n");
   assert(fd_dir(0) == 3);
   printf("============================================================"
          "==========\n");
   printf ("Typing FILE2.TXT\n");
   printf("============================================================"
          "==========\n");
   assert(fd_type("FILE2.TXT") == 9062);
   assert(fd_cd("..") == 0);
   assert(fd_cd("..") == 0);
   printf("============================================================"
          "==========\n");
   printf ("NEW sub-directory listing\n");
   printf("============================================================"
          "==========\n");
   assert(fd_dir(0) == 23);
   assert(fd_cd("..") == 0);
   assert(fd_cd("WINDOWS") == 0);
   printf("============================================================"
          "==========\n");
   printf ("WINDOWS sub-directory listing\n");
   printf("============================================================"
          "==========\n");
   assert(fd_dir(0) == 6);
   assert(fd_cd("..") == 0);
   assert(fd_cd("..") == 0);
   assert(fd_cd("NEW") == 0);

#ifdef TEST_WRITES

   printf("============================================================"
          "==========\n");
   printf ("Testing the John Sturgis condition\n");
   printf("============================================================"
          "==========\n");
   assert(fd_creat("JOHNSTUR.GIS") == 0);
   assert(fd_del("JOHNSTUR.GIS") == 0);
   printf("============================================================"
          "==========\n");
   printf ("Creating 10 files in NEW\n");
   printf("============================================================"
          "==========\n");
   assert(fd_creat("CREATE00.TXT") == 0);
   assert(fd_creat("CREATE01.TXT") == 0);
   assert(fd_creat("CREATE02.TXT") == 0);
   assert(fd_creat("CREATE03.TXT") == 0);
   assert(fd_creat("CREATE04.TXT") == 0);
   assert(fd_creat("CREATE05.TXT") == 0);
   assert(fd_creat("CREATE06.TXT") == 0);
   assert(fd_creat("CREATE07.TXT") == 0);
   assert(fd_creat("CREATE08.TXT") == 0);
   assert(fd_creat("CREATE09.TXT") == 0);
   assert(fd_dir(0) == 33);
   printf("============================================================"
          "==========\n");
   printf ("Appending 1133 bytes of data to CREATE00.TXT\n");
   printf("============================================================"
          "==========\n");
   assert(fd_append("CREATE00.TXT", "This is some data.\r\n", 20) == 20);
   assert(fd_append("CREATE00.TXT", "This is some more data.\r\n", 25) == 25);
   assert(fd_append("CREATE00.TXT",
                    "00                           ;\r\n"
                    "01                           ;\r\n"
                    "02                           ;\r\n"
                    "03                           ;\r\n"
                    "04                           ;\r\n"
                    "05                           ;\r\n"
                    "05                           ;\r\n"
                    "07                           ;\r\n"
                    "08                           ;\r\n"
                    "09                           ;\r\n"
                    "10                           ;\r\n"
                    "11                           ;\r\n"
                    "12                           ;\r\n"
                    "13                           ;\r\n"
                    "14                           ;\r\n"
                    "15                           ;\r\n", 512) == 512);
   assert(fd_append("CREATE00.TXT",
                    "16                           ;\r\n"
                    "17                           ;\r\n"
                    "18                           ;\r\n"
                    "19                           ;\r\n"
                    "20                           ;\r\n"
                    "21                           ;\r\n"
                    "22                           ;\r\n"
                    "23                           ;\r\n"
                    "24                           ;\r\n"
                    "25                           ;\r\n"
                    "26                           ;\r\n"
                    "27                           ;\r\n"
                    "28                           ;\r\n"
                    "29                           ;\r\n"
                    "30                           ;\r\n"
                    "31                           ;\r\n"
                    "32                           ;\r\n"
                    "33                           ;\r\n", 576) == 576);
   assert(fd_type("CREATE00.TXT") == 1133);

   assert(fd_del("TROUBLE.TXT") == -1);
   assert(fd_cd("..") == 0);
   assert(fd_del("TROUBLE.TXT") == 35);

   printf("\n\nRun\n"
          "   /sbin/dosfsck -v floppyData.img\n"
          "and check for errors.\n");
   printf("The last two lines of output should be:\n"
          "   Checking for unused clusters.\n"
          "   floppyData.img: 66 files, 2719/2847 clusters\n");

#endif

   assert(fd_unmount(dev) != -1);

   return 0;
}
