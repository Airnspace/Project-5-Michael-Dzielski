/***********************************************************************
 * shell.c
 *
 * Tom Kelliher, Goucher College (c) 2016
 *
 * A simple interactive shell program for interacting with the DOS
 * FAT12 file system operations for Project 5.
 ***********************************************************************/


#include <stdio.h>
#include <string.h>
#include "fsops.h"


#define COMMAND_LEN 1024
#define DELIMS " \n\r\r"
#define MAX_TOKENS 3
#define FILE_SIZE_LIMIT 64 * 1024


void listHelp(void);
int appendf(const char *destFile, const char *srcFile);
char *getStringArg(char *cmd);


/***********************************************************************
 * main
 ***********************************************************************/

int main(int argc, char *argv[]) {
   int dev;
   char command[1024];
   char *tokens[MAX_TOKENS];

   if (argc != 2) {
      printf("File name for floppy image expected.\n");
      return -1;
   }

   if ((dev = fd_mount(argv[1])) == -1) {
      printf("Couldn't mount floppy image.\n");
   }

   listHelp();

   while (1) {
      printf("> ");

      /* Check for an empty input line. */

      if (*fgets(command, COMMAND_LEN, stdin) == '\n')
         continue;

      /* Tokenize the command read from stdin. */

      tokens[0] = strtok(command, DELIMS);

      if (strcmp(tokens[0], "appends") == 0) {
         tokens[1] = strtok(NULL, DELIMS);
         tokens[2] = getStringArg(command);
         if (tokens[2] != NULL)
            strcat(tokens[2], "\n");
      }
      else {
         tokens[1] = strtok(NULL, DELIMS);
         tokens[2] = NULL;
      }

      if (strcmp(tokens[0], "appendf") == 0)
         tokens[2] = strtok(NULL, DELIMS);

      /* Actual command parsing begins here. */

      if (strcmp(tokens[0], "help") == 0)
         listHelp();
      else if (strcmp(tokens[0], "exit") == 0)
         break;
      else if (strcmp(tokens[0], "dir") == 0)
         printf("\nReturn value: %d\n", fd_dir(tokens[1] != NULL));
      else if (strcmp(tokens[0], "cd") == 0)
         printf("\nReturn value: %d\n", fd_cd(tokens[1]));
      else if (strcmp(tokens[0], "type") == 0)
         printf("\nReturn value: %d\n", fd_type(tokens[1]));
      else if (strcmp(tokens[0], "del") == 0)
         printf("\nReturn value: %d\n", fd_del(tokens[1]));
      else if (strcmp(tokens[0], "creat") == 0)
         printf("\nReturn value: %d\n", fd_creat(tokens[1]));
      else if (strcmp(tokens[0], "appends") == 0)
         printf("\nReturn value: %d\n", fd_append(tokens[1], tokens[2],
                                                  strlen(tokens[2])));
      else if (strcmp(tokens[0], "appendf") == 0)
         printf("\nReturn value: %d\n", appendf(tokens[1], tokens[2]));
      else
         printf("Unrecognized command: %s\n", tokens[0]);
   }

   fd_unmount(dev);
   return 0;
}


/* List the commands available.  Maybe I should have named this
 * listCommands()?
 */

void listHelp(void) {
   printf("\nValid commands are:\n");
   printf("\n   help\n");
   printf("\n   exit\n");
   printf("\n   dir [/h]\n");
   printf("      /h --- list hidden files.\n");
   printf("\n   cd directory\n");
   printf("\n   type file\n");
   printf("\n   del file\n");
   printf("\n   creat file\n");
   printf("\n   appends file stringToAppend\n");
   printf("      stringToAppend should be delimited by quotes "
          "and not contain quotes.\n");
   printf("      A new line character will be appended to the string.\n");
   printf("\n   appendf destFile srcFile\n");
   printf("      srcFile should exist in the host file system\n\n");
}


/* Handle appending a file in the host file system to a file in the
 * floppy image.
 */

int appendf(const char *destFile, const char *srcFile) {
   FILE *src;
   char buffer[FILE_SIZE_LIMIT];
   int count = 0;

   if ((src = fopen(srcFile, "r")) == NULL)
       return 0;

   /* Read the source file into a buffer. */

   while (count < FILE_SIZE_LIMIT && (buffer[count] = fgetc(src)) != EOF)
      count++;

   fclose(src);
   return fd_append(destFile, buffer, count);
}


/* Search for a quoted string within cmd.
 *
 * Assumes that the string itself contains no quotes.
 *
 * Returns the string, sans the quote characters.  Returns NULL if a
 * quoted string can't be found.
 */

char *getStringArg(char *cmd) {
   int index = 0;
   char *begin = cmd;
   char *end;

   /* Look for the first quote character.  Try to avoid running off the
    * end of cmd.
    */

   while (index < COMMAND_LEN && *begin != '\"') {
      begin++;
      index++;
   }

   /* Advance past the quote character. */

   end = ++begin;
   index++;

   /* Look for the second quote character. */

   while (index < COMMAND_LEN && *end != '\"') {
      end++;
      index++;
   }

   if (index >= COMMAND_LEN)
      return NULL;
   else {
      *end = '\0';   /* Replacing the second quote character. */
      return begin;
   }
}
