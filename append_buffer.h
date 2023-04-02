#ifndef APPEND_BUFFER_H
#define APPEND_BUFFER_H

#include <string.h>
#include <stdlib.h>

#define ABUF_INIT {NULL, 0}

struct appendBuffer{
    char *buffer;
    int len;
};

void abAppend(struct appendBuffer*, const char*, int);
void abFree(struct appendBuffer*);

#endif
