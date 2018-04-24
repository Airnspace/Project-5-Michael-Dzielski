This repository contains the starting code for the FAT12 Filesystem Project.


The shell program may be built simply by running

   make

from the command line.


Don't forget to perform final testing with the exercise program, and see
the comment in exercise.c for the TEST_WRITES #define.  To build the
exercise program, run

   make exercise

from the command line.  If exercise is compiled and run with TEST_WRITES
defined, run

   git checkout -- floppyData.img

to restore the floppy image to its original state after each run.


Run

   make exercise2

from the command line to build exercise2.  Run

   git checkout -- floppyData.img

to restore the floppy image to its original state after each run.
