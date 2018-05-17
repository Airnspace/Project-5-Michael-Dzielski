/***********************************************************************
 * fsops.c
 *
 * Tom Kelliher, Goucher College (c) 2016
 *
 * DOS FAT12 filesystem operations for Project 5.  Refer to the
 * documentation below and to the project description.
 ***********************************************************************/


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "fstypes.h"
#include "fsops.h"


/* Private global variables. */

/* In-memory cached copy of floppy image's FAT. */
static fat_t g_fat;
/* In-memory cached copy of floppy images's root directory. */
static root_t g_root;
/* if cwdHead == 0, the root directory is the current working directory.
 * Otherwise, cwdHead holds the logical block number of the first
 * block of the current working directory.
 */
static unsigned int g_cwdHead = 0;
/* Device number of mounted floppy disk image. */
static int g_dev = -1;


/* Prototypes for private helper functions.  Prototypes for public
 * API functions should be in fsops.h
 */
static int fd_dir_root(int showAll);
static int fd_dir_subdir(int showAll);
static int direntryFree(const direntry_t *direntry);
static int hidden(const direntry_t *direntry);
static int subdirectory(const direntry_t *direntry);
static int longFN(const direntry_t *direntry);
static void list(const direntry_t *direntry);
static char *getfilename(const direntry_t *direntry, char *fn);
static void putdirentry(direntry_t *direntry, const char *fn,
                        unsigned int attrib, struct tm *time,
                        unsigned int strtBlk, unsigned int size);
static struct tm *getTime(void);
static direntry_t *searchRoot(const char *name);
static direntry_t *searchSubdir(const char *name, block_t block,
                                unsigned int *blkindex);
static direntry_t *getFreeRootEntry(void);
static direntry_t *getFreeSubDirEntry(block_t block,
                                      unsigned int *blkindex);
static unsigned int ltop(unsigned int lblock);
static unsigned int getFreeFatEntry(const fat_t fat);
static unsigned int getfatentry(const fat_t fat, unsigned int index);
static void putfatentry(fat_t fat, unsigned int index, unsigned int val);
static int lastBlk(unsigned int blknum);
static int cwdIsRoot(void);


/* Mount a floppy disk image.  img is the image's file name.  This
 * function also caches the image's FAT and root directory in private
 * global variables fat and root.
 *
 * Returns the device number on success.  Otherwise, it returns -1.
 */
int fd_mount(const char *img)
{
   int i;
   /* readblock() works with blocks.  The following blocks variable is
    * used to treat the fat and root caches as arrays of blocks.
    */
   block_t *blocks;

   if (g_dev != -1 || (g_dev = fdimgopen(img)) == -1)
      return -1;

   /* Cache the first FAT */
   blocks = (block_t *) g_fat;
   for (i = 0; i < FAT_BLOCKS; i++)
      readblock(g_dev, blocks[i], i + FAT1_START);

   /* Cache the root directory */
   blocks = (block_t *) g_root;
   for (i = 0; i < ROOT_BLOCKS; i++)
      readblock(g_dev, blocks[i], i + ROOT_START);

   return g_dev;
}



/* Unmount the floppy disk image with device number dev.  This function
 * flushes the cached FAT and root directory to the image file before
 * unmounting it.
 *
 * Returns 0 on success.  Otherwise, it returns -1;
 */
int fd_unmount(int dev)
{
   int i;
   int devTmp = dev;
   /* writeblock() works with blocks.  The following blocks variable is
    * used to treat the fat and root caches as arrays of blocks.
    */
   block_t *blocks;

   /* Write both FATs */
   blocks = (block_t *) g_fat;
   for (i = 0; i < FAT_BLOCKS; i++)
   {
      writeblock(dev, blocks[i], i + FAT1_START);
      writeblock(dev, blocks[i], i + FAT2_START);
   }

   /* Write the root directory */
   blocks = (block_t *) g_root;
   for (i = 0; i < ROOT_BLOCKS; i++)
      writeblock(dev, blocks[i], i + ROOT_START);

   g_dev = -1;
   return fdimgclose(devTmp);
}


/* List the entries in the current working directory.  Hidden entries
 * are listed if showAll is true, otherwise hidden entries are not listed.
 * Entries with long file names are never listed.
 *
 * After the individual entries are listed, a summary line showing the number
 * of listed entries and the total number of bytes of all the files listed
 * is printed.
 *
 * Returns the number of entries listed.
 */
int fd_dir(int showAll)
{

  if (cwdIsRoot()){
    return fd_dir_root(showAll);
   }
  else{
    fd_dir_subdir(showAll);
  }
   return -1;
}



/* Change the current working directory to the parent directory
 * (dir == "..") else to the sub-directory dir.
 *
 * The root directory is its own parent.
 *
 * Because dir might contain lower case characters, toupper() should be used
 * to convert all characters to upper case.  Do _not_ modify the string
 * pointed to by dir.  The "const" attribute attached to dir causes the
 * compiler to enforce this restriction.
 *
 * If the first character of dir is 0xe5, return -1.  If dir does not
 * correspond to a directory, return -1.
 *
 * Returns 0 on success, otherwise -1.
 */
int fd_cd(const char *dir)
{
  char sstring[16];
  int i;
  block_t block;
  unsigned int bi;
  direntry_t *direntry;
  //prevents segmentation faults
  if(dir == NULL) return -1;
  //in case of edge cases
  if ((unsigned char) dir[0] == (unsigned char) 0xe5) return -1;
  //doesn't change directory because the root is it's own parent
  if (strcmp(dir, "..") == 0 && cwdIsRoot()) return 0;

  //turns string into an uppercase copy of dir
  strcpy(sstring, dir);
  for (i = 0; i < 16; i++){
    sstring[i] = toupper(sstring[i]);}

  if(cwdIsRoot()){
    direntry = searchRoot(sstring);}
  else direntry = searchSubDir(sstring, block, &bi);

  if (direntry == NULL) return -1;
  if (!subdirectory(direntry)) return -1;

  g_cwdHead = direntry->firstSector;

  return 0;

}



/* Type (list) the contents of file, located in the current working
 * directory.
 *
 * Because file might contain lower case characters, toupper() should be used
 * to convert all characters to upper case.  Do _not_ modify the string
 * pointed to by dir.  The "const" attribute attached to dir causes the
 * compiler to enforce this restriction.
 *
 * If the first character of file is 0xe5, return -1.  If the file
 * corresponds to a directory, return -1.
 *
 * On success, returns the number of characters typed.  Otherwise, returns -1.
 */
int fd_type(const char *file)
{
  char string[16];
  int i = 0;
  int sec = 0;
  int fsize = 0;
  int nchar = 0;
  block_t block;
  unsigned int bindex;
  direntry_t *direntry;

  //in case of edge cases
  if (file == NULL) return -1;
  if ((unsigned char) file[0] == (unsigned char) 0xe5) return-1;

  //turns string into an uppercase copy of the file
  strcpy(string, file);
  for (i = 0; i < 16; i++) string[i] = toupper(string[i]);
  //searches in the direnty for the appropriate place
  if (cwdIsRoot()) direntry = searchRoot(string);
  else direntry = searchSubDir(string, block, &bindex);
  //checks for failure
  if (direntry == NULL) return -1;
  if (subdirectory(direntry)) return -1;
  //reads the file
  sec = direntry->firstSector;
  fsize = direntry->fileSize;

  if (!sec) return -1;
  //keeps reading while there is still more to read
  while(!lastBlk(sec)){
    readblock(g_dev, block, ltop(sec));
    sec = getfatentry(g_fat, sec);
    for (i=0; i < BLOCKSIZE && nchar < fsize; i++) {
      putchar(block[i]);
      nchar++;
    }
  }
  return nchar;
}


/* Delete the file in the current working directory named file.  Its
 * directory entry should be marked free and the blocks allocated to 
 * it should also be marked free.  If the freed directory entry is in a
 * sub-directory, the modified block must be written back to disk
 * immediately.
 *
 * Because file might contain lower case characters, toupper() should be used
 * to convert all characters to upper case.  Do _not_ modify the string
 * pointed to by dir.  The "const" attribute attached to dir causes the
 * compiler to enforce this restriction.
 *
 * If the first character of file is 0xe5, return -1.  If file corresponds
 * to a directory, return -1.
 *
 * On success, return the number of blocks freed.  Otherwise, return
 * -1.
 */
int fd_del(const char *file){


   return -1;
}


/* Create a new file with the name passed in the 8.3 format string parameter
 * file in the current working directory.  The file's attributes should be
 * set to 0.  The creation, write, and access times should be set to the
 * current time in the current time zone.  The file's starting block should
 * be set to 0 and its size should be set to 0 bytes.  See the Section 2
 * man page for time() and the manpage for localtime() for getting the
 * current time and converting it to the current time zone.  Or, use the
 * getTime() helper function.
 *
 * Because file might contain lower case characters, toupper() should be used
 * to convert all characters to upper case.  Do _not_ modify the string
 * pointed to by dir.  The "const" attribute attached to dir causes the
 * compiler to enforce this restriction.
 *
 * The 8.3 format file name in file should be converted to upper case.
 *
 * Returns -1 if the first character of file is 0xe5, if the directory
 * already contains an entry with the same filename, or if the directory
 * entry can't be created.  Otherwise, returns 0.
 */
int fd_creat(const char *file)
{
  int i;
  char string[16];
  block_t block;
  unsigned int blkindex;
  direntry_t *direntry;
  time_t t;
  //edge casews
  if (file == NULL) return -1;
  if ((unsigned char) file[0] == (unsigned char) 0xe5) return -1;
  //translates again to uppercase
  strcpy(string, file);
  for (i=0; i < 16; i++) {
    string[i] = toupper(file[i]); }
  //makes sure there's no other files or subdirectories with the same name
  if (cwdIsRoot()) direntry = searchRoot(string);
  else direntry = getFreeSubDirEntry(block, &blkindex);

  if (direntry == NULL) return -1;
  t = time(NULL);
  
  putdirentry(direntry, string, 0, localtime(&t), 0, 0);
  if(!cwdIsRoot()){
    writeblock(g_dev, block, ltop(blkindex));
  }

   return 0;
}


/* Append len characters of data to file in the current working directory.
 *
 * Because file might contain lower case characters, toupper() should be used
 * to convert all characters to upper case.  Do _not_ modify the string
 * pointed to by dir.  The "const" attribute attached to dir causes the
 * compiler to enforce this restriction.
 *
 * If the first character of file is 0xe5, return -1.  If the file
 * corresponds to a sub directory, return -1.
 *
 * Returns the number of characters appended to the file.
 */
int fd_append(const char *file, const char *data, unsigned int len)
{
   return -1;
}


/* Private helper functions follow.
 */


/* List the entries in the root directory.  Called by fd_dir().
 *
 * Returns the number of entries listed.
 */
static int fd_dir_root(int showAll)
{
 int i;
   direntry_t *direntry = (direntry_t *) g_root;
   char name2[16];
   int count;
   int fsize;

   for (i = 0; i < ROOT_BLOCKS * DIR_ENTRIES; i++, direntry++)
     if (!longFN(direntry)  && (!hidden(direntry) || showAll) && !direntryFree(direntry)){
      list(direntry);
      fsize += direntry ->fileSize;
      count++;
    }
   printf("# of Entries: %d\n# Bytes: %d\n", count, fsize);
   return count;
}


/* List the entries in a sub-directory.  Called by fd_dir().
 *
 * Returns the number of entries listed.
 */
static int fd_dir_subdir(int showAll)
{
     int i;
     int count = 0;
     int size = 0;
   unsigned int blk = g_cwdHead;
   direntry_t *direntry;
   block_t block;


   while (!lastBlk(blk))
   {
      readblock(g_dev, block, ltop(blk));
      direntry = (direntry_t *) block;

      for (i = 0; i < DIR_ENTRIES; i++, direntry++)
	{
	if ((showAll || !hidden(direntry  )) && !longFN(direntry  ) && !direntryFree(direntry  ))
         {
	   list(direntry  );
	   size += direntry ->fileSize;
	   count++;
         }
	}
      blk = getfatentry(g_fat, blk);
   }
   printf("# Entries: %d\n# Bytes: %d\n", count, size);
   return count;
}


/* Returns 1 if the directory entry pointed to by direntry is free.
/ * Otherwise, returns 0.
 */
static int direntryFree(const direntry_t *direntry)
{
   if (direntry->filename[0] == (unsigned char) 0xe5
       || direntry->filename[0] == 0x00)
      return 1;
   else
      return 0;
}


/* Returns 1 if the directory entry pointed to by direntry is hidden.
 * Otherwise, returns 0.
 */
static int hidden(const direntry_t *direntry)
{

   return direntry->attributes & HIDDEN;
}


/* Returns 1 if the directory entry pointed to by direntry is a
 * sub-directory.  Otherwise, returns 0.
 */
static int subdirectory(const direntry_t *direntry)
{
   return direntry->attributes & SUBDIRECTORY;
}


/* Returns 1 if the directory entry pointed to by direntry contains
 * a long filename.  Otherwise, returns 0.
 */
static int longFN(const direntry_t *direntry)
{
   return (direntry->attributes & 0XF) == 0XF;
}


/* List the directory entry pointed to by direntry.
 */
static void list(const direntry_t *direntry)
{
   char filename[16];

   printf("%12s  %8d   %8d   %8x %4d-%02d-%02d  %2d:%02d:%02d\n",
          getfilename(direntry, filename),
          direntry->firstSector,
          direntry->fileSize,
          direntry->attributes,
          /* Years field, bits 9--15, is years since 1980. */
          ((direntry->lastWriteDate >> 9) & 0X7F) + 1980,
          /* Months field is bits 5--8. */
          (direntry->lastWriteDate >> 5) & 0XF,
          /* Day of month field is bits 0--4. */
          direntry->lastWriteDate & 0X1F,
          /* Hours field, in 24 hour format,  is bits 11--15. */
          (direntry->lastWriteTime >> 11) & 0X1F,
          /* Minutes field is bits 5--10. */
          (direntry->lastWriteTime >> 5) & 0X3F,
          /* Seconds field is bits 0--4.  Granularity is 2 seconds
           * because we're one bit short of the number of bits needed to
           * represent a number in the range [0--59].
           */
          (direntry->lastWriteTime & 0X1F) << 1);
}


/* Get the filename from the directory entry pointed to by direntry.
 * The filename will be stored in fn, which should point to sufficient
 * storage to store the filename in 8.3 format as a null-terminated string.
 *
 * Returns fn.
 */
static char *getfilename(const direntry_t *direntry, char *fn)
{
   int i;
   char *ptr = fn;

   i = 0;

   while ((*ptr = direntry->filename[i]) != ' ' && i++ < 8)
      ptr++;

   if (direntry->extension[0] == ' ')
   {
      *ptr = '\0';
      return fn;
   }

   *ptr++ = '.';
   i = 0;

   while ((*ptr = direntry->extension[i]) != ' ' && i++ < 3)
      ptr++;

   *ptr = '\0';

   return fn;
}


/* Set the directory entry pointed to by direntry to the name fn, with
 * attributes attrib.  entryTime will be used to set the creation, access,
 * and write dates/times.  The entry's starting block will be set to
 * strtBlk and the entry's size will be set to size.
 *
 * fn should be in 8.3 format.  Characters will be converted to upper case.
 */
static void putdirentry(direntry_t *direntry, const char *fn,
                        unsigned int attrib, struct tm *entryTime,
                        unsigned int strtBlk, unsigned int size)
{
   int i;
   int j;
   unsigned int packedTime;
   unsigned int packedDate;

   /* Initialize and then set the filename and extension fields. */

   for (i = 0; i < 8; i++)
      direntry->filename[i] = ' ';

   for (i = 0; i < 3; i++)
      direntry->extension[i] = ' ';

   i = 0;

   while (i < 8 && fn[i] != '.' && fn[i] != '\0')
   {
      direntry->filename[i] = toupper(fn[i]);
      i++;
   }

   /* Search for the extension. */

   while (fn[i] != '.' && fn[i] != '\0')
      i++;

   if (fn[i] == '.')   // Found it.
   {
      i++;
      j = 0;

      while (j < 3 && fn[i] != '\0')
      {
         direntry->extension[j] = toupper(fn[i]);
         i++;
         j++;
      }
   }

   direntry->attributes = 0xff & attrib;

   packedTime = ((0x1f & entryTime->tm_hour) << 11) |
      ((0x3f & entryTime->tm_min) << 5) |
      (0x1f & (entryTime->tm_sec >> 1));
   direntry->creationTime = packedTime;
   direntry->lastWriteTime = packedTime;

   packedDate = ((0x7f & (entryTime->tm_year - 80)) << 9) |
      ((0xf & (entryTime->tm_mon + 1)) << 5) |
      (0x1f & entryTime->tm_mday);
   direntry->creationDate = packedDate;
   direntry->lastAccess = packedDate;
   direntry->lastWriteDate = packedDate;

   direntry->firstSector = 0xffff & strtBlk;
   direntry->fileSize = 0xffffffff & size;
}


/* Get the current time and convert to current time in the local
 * time zone.
 *
 * Returns a struct tm pointer which can be used as the entryTime parameter
 * for putdirentry().  This pointer points to a statically allocated struct
 * which could be overwritten by subsequent calls to any of the time and
 * date functions.  Hence, this function is not thread-safe.
 */
static struct tm *getTime(void)
{
   time_t now;
   time(&now);
   return localtime(&now);
}


/* Search the root directory for an entry with a file name of name.
 * name should point to a null-terminated string holding an 8.3 format
 * file name.
 *
 * Ignore directory entries containing long file names.
 *
 * On success, returns a pointer to the directory entry.  Otherwise, return
 * NULL.
 */
static direntry_t *searchRoot(const char *name)
{
   int i;
   direntry_t *direntry = (direntry_t *) g_root;
   char name2[16];

   for (i = 0; i < ROOT_BLOCKS * DIR_ENTRIES; i++, direntry++)
      if (!longFN(direntry)
          && strcmp(name, getfilename(direntry, name2)) == 0)
         return direntry;

   return NULL;
}


/* Search a sub-directory for an entry with a file name of name.
 * name should point to a null-terminated string holding an 8.3 format
 * file name.  The current working directory is the directory that will
 * be searched.  Block should point to a variable of type block_t.
 * blkindex should point to a variable of type  unsigned int.
 *
 * Ignore directory entries containing long file names.
 *
 * On success, returns a pointer to the directory entry.  The block
 * pointed to by block will be written with the disk block containing the 
 * directory entry and the unsigned int pointed to by blkindex will
 * contain the logical block number of this block.  The directory
 * entry pointer returned by this function will point into this block.
 * On failure, return NULL.
 */
static direntry_t *searchSubdir(const char *name, block_t block,
                                unsigned int *blkindex)
{
   return NULL;
}


/* Search for a free entry in the root directory, stored in the global
 * variable root.
 *
 * Returns a pointer to the first free entry.  If there are not free
 * entries, returns NULL.
 */
static direntry_t *getFreeRootEntry(void)
{
   return NULL;
}


/* Search for a free entry in the sub-directory whose first block is
 * pointed to by cwdHead.  block should be a pointer to a variable of type
 * block_t and blkindex should be a pointer to a variable of type unsigned
 * int.
 *
 * If there is no free entry in the blocks currently allocated to the
 * sub-directory, this function will attempt to allocate a new block
 * for the sub-directory, initialize it, and return a pointer to the first
 * entry in the new block.
 *
 * Returns a pointer to the first free directory entry.  This pointer
 * will point into block, and the variable pointed to by blkindex will
 * contain the block's logical block number.  If no new entry can be found
 * or created, returns NULL.
 */
static direntry_t *getFreeSubDirEntry(block_t block,
                                      unsigned int *blkindex)
{
   int i;
   unsigned int free;
   unsigned int blk = g_cwdHead;
   unsigned int oldBlk;
   direntry_t *direntry;

   /* Search the sub-directory's existing blocks. */

   while (!lastBlk(blk))
   {
      readblock(g_dev, block, ltop(blk));
      direntry = (direntry_t *) block;

      for (i = 0; i < DIR_ENTRIES; i++, direntry++)
         if (direntryFree(direntry))
         {
            *blkindex = blk;
            return direntry;
         }

      oldBlk = blk;
      blk = getfatentry(g_fat, blk);
   }

   /* If we've arrived here, there are no free entries in the
    * sub-directory's existing blocks.  Attempt to allocate and
    * initialize a new block for the sub-directory.
    */

   if ((free = getFreeFatEntry(g_fat)) == 0)   // Allocation failed.
      return NULL;

   /* Allocation succeeded.  Link the new block and set it as the
    * sub-directory's last block.
    */

   putfatentry(g_fat, oldBlk, free);
   putfatentry(g_fat, free, 0xfff);

   direntry = (direntry_t *) block;

   for (i = 0; i < DIR_ENTRIES; i++, direntry++)
      direntry->filename[0] = 0x00;

   writeblock(g_dev, block, ltop(free));
   *blkindex = free;
   return (direntry_t *) block;
}


/* Convert a logical block number to a physical block number.
 */
static unsigned int ltop(unsigned int lblock)
{
   return lblock + 31;
}


/* Search for a free FAT entry in fat.
 *
 * Returns the index of the first free FAT entry.  If no free entry can be
 * found, returns 0.  (FAT entry 0 is reserved.  Hence, 0 amounts to an
 * invalid FAT index.
 */
static unsigned int getFreeFatEntry(const fat_t fat)
{
   unsigned int i;

   for (i = 2; i < (FAT_BLOCKS * BLOCKSIZE * 2) / 3; i++)
      if (getfatentry(fat, i) == 0)
         return i;

   return 0;
}



/* Return the FAT entry at the given index within fat.
 */
static unsigned int getfatentry(const fat_t fat, unsigned int index)
{
   /* FAT entries straddle consecutive bytes in the FAT.  offset
    * is the position of the first byte of the FAT entry of interest.
    */
   unsigned int offset = (3 * index) >> 1;
   /* Treat the FAT as an array of bytes. */
   const uint8_t *fatbase = (const uint8_t *) fat;
   unsigned int ret;
   unsigned int temp;
   
   if ((index & 0x1) == 0) // Index is even
   {
      ret = (unsigned int) fatbase[offset];
      temp = (0xf & (unsigned int) fatbase[offset + 1]) << 8;
      ret |= temp;
   }
   else
   {
      ret = ((unsigned int) fatbase[offset]) >> 4;
      temp = ((unsigned int) fatbase[offset + 1]) << 4;
      ret |= temp;
   }

   return ret;
}


/* Write val to the FAT entry at the given index within fat.
 */
static void putfatentry(fat_t fat, unsigned int index, unsigned int val)
{
   /* FAT entries straddle consecutive bytes in the FAT.  offset
    * is the position of the first byte of the FAT entry of interest.
    */
   unsigned int offset = (3 * index) >> 1;
   /* Treat the FAT as an array of bytes. */
   uint8_t *fatbase = (uint8_t *) fat;
   
   if ((index & 0x1) == 0) // Index is even
   {
      fatbase[offset] = (uint8_t) (0xFF & val);
      fatbase[offset + 1] &= (uint8_t) 0XF0;
      fatbase[offset + 1] |= (uint8_t) (0X0F & (val >> 8));
   }
   else
   {
      fatbase[offset] &= (uint8_t) 0X0F;
      fatbase[offset] |= (uint8_t) ((val << 4) & 0xF0);
      fatbase[offset + 1] = (uint8_t) (0xFF & (val >> 4));
   }
}


/* Return 1 if the block number value blknum corresponds to the last block
 * of a file.  Otherwise, return 0.
 */
static int lastBlk(unsigned int blknum)
{
   if (0XFF8 <= blknum && blknum <= 0XFFF)
      return 1;
   else
      return 0;
}


/* Returns 1 if the current working directory is the root directory.
 * Otherwise, returns 1.
 */
static int cwdIsRoot(void)
{
   return g_cwdHead == 0;
}
