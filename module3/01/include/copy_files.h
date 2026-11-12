#ifndef COPY_FILES_H
#define COPY_FILES_H

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define BLOCK 4096
#define NAMELEN 256

int child_proc(int, int);
int parent_proc(int, int, int, char **);

#endif