/***********************************************************************
 * fsops.h
 *
 * Make no changes to this file!
 *
 * Refer to the Project 5 description for details and to fsops.c for
 * descriptions of these public API functions.
 ***********************************************************************/


#ifndef __FSOPS_H
#define __FSOPS_H


/* Function prototypes */
int fd_mount(const char *img);
int fd_unmount(int dev);
int fd_dir(int showAll);
int fd_cd(const char *dir);
int fd_type(const char *file);
int fd_del(const char *file);
int fd_creat(const char *file);
int fd_append(const char *file, const char *data, unsigned int len);


#endif
