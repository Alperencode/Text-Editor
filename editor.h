#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "append_buffer.h"
#include "raw_mode.h"

enum editorKeys{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
};

struct editorConfig{
    int screenRows, screenCols;
    int cursorX, cursorY;
    struct termios original_termios;
};

struct editorConfig Editor;

int editorReadKey();
void editorProcessKeypress();
void editorDrawRows();
void editorRefreshScreen();
void editorMoveCursor(int);
void initEditor();
int getWindowSize(int*, int*);

void clearScreen();
void die(const char*);

#endif
