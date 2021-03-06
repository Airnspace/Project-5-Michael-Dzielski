CC = gcc

CFLAGS = -g -std=c99 -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual \
         -Wstrict-prototypes -Wmissing-prototypes -Wno-unused-function

SOURCES = fsops.c driver.c
BINARIES = shell exercise exercise2

shell: shell.c $(SOURCES)
	$(CC) $(CFLAGS) shell.c $(SOURCES) -o shell

exercise: exercise.c $(SOURCES)
	$(CC) $(CFLAGS) exercise.c $(SOURCES) -o exercise

exercise2: exercise2.c $(SOURCES)
	$(CC) $(CFLAGS) exercise2.c $(SOURCES) -o exercise2

all: shell.c exercise.c exercise2.c $(SOURCES)
	$(CC) $(CFLAGS) shell.c $(SOURCES) -o shell
	$(CC) $(CFLAGS) exercise.c $(SOURCES) -o exercise
	$(CC) $(CFLAGS) exercise2.c $(SOURCES) -o exercise2

rfd:
	git checkout -- floppyData.img

clean:
	/bin/rm -f $(BINARIES)
