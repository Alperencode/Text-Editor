#ifndef EDITOR_H
#define EDITOR_H

/** Includes **/
#include <termios.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "append_buffer.h"

/** Macros **/
#define VERSION "0.0.1"
#define CTRL_KEY(c) ((c) & 0x1f) /* ands with 0x1f the given key (what ctrl key does) */

/** Function Prototypes **/
char editorReadKey();
void editorProcessKeypress();
void editorDrawRows();
void editorRefreshScreen();
void editorMoveCursor(char);
void enableRawMode();
void disableRawMode();
void initEditor();
int getWindowSize(int*, int*);
void die(const char*);

struct editorConfig{
    int screenRows, screenCols;
    int cursorX, cursorY;
    struct termios original_termios;
};

struct editorConfig Editor;

/**
 * Waits for the user to press a key and returns the key that was pressed.
 *
 * @return The key that was pressed.
 */
char editorReadKey() {
    int readReturn;
    char c;

    // Wait for a single keypress from the user.
    while ((readReturn = read(STDIN_FILENO, &c, 1)) != 1) {
        if (readReturn == -1) die("read error");
    }

    return c;
}

/**
 * Processes a single keypress from the user and performs the appropriate action.
 */
void editorProcessKeypress() {
    char c = editorReadKey();

    // If the user pressed Ctrl-Q, exit the program.
    if (c == CTRL_KEY('q'))
        die("Exit Program");
    else if(c == 'w' || 's' || 'a' || 'd')
        editorMoveCursor(c);

}

/**
 * Draws the rows of the editor to the screen.
 */
void editorDrawRows(struct appendBuffer *ab) {
    int i;

    // Write a tilde character to each row of the screen, followed by a newline character.
    for (i = 0; i < Editor.screenRows; i++){

        if(i == Editor.screenRows / 2){
            // Printing Welcome message
            char welcomeMessage[35];
            abAppend(ab, "~", 1);

            // Interpolate welcome message with Version
            int welcomeMessageLen = snprintf(
                    welcomeMessage,
                    sizeof(welcomeMessage),
                    "Text Editor -- Version %s",
                    VERSION);

            if(welcomeMessageLen > Editor.screenCols)
                welcomeMessageLen = Editor.screenCols;

            // Centering the welcome message
            int padding = (Editor.screenCols - welcomeMessageLen) / 2;
            while(--padding) abAppend(ab, " ", 1);

            // Sending message to buffer
            abAppend(ab, welcomeMessage, welcomeMessageLen);
        }else
            // Printing tilde for each line
            abAppend(ab, "~", 1);

        // Sending erase line command with escape seuqence for each line
        abAppend(ab, "\x1b[K", 3);

        if(i < Editor.screenRows -1)
            abAppend(ab, "\r\n", 2);

    }

}


/**
 * Clears the screen and redraws the editor contents.
 */
void editorRefreshScreen() {
    struct appendBuffer ab = ABUF_INIT;

    // Send ANSI escape sequences to hide cursor and reset the cursor position.
    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3);

    // Redraw the editor rows.
    editorDrawRows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", Editor.cursorY + 1, Editor.cursorX + 1);
    abAppend(&ab, buf, strlen(buf));

    // Show the cursor.
    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.buffer, ab.len);
    abFree(&ab);
}

/**
 * Moves the cursor position in response to a user input key.
 *
 * @param key The key char that was pressed to trigger the cursor movement.
 */
void editorMoveCursor(char key) {

    switch(key) {
        case 'a':
            Editor.cursorX--;
            break;
        case 'd':
            Editor.cursorX++;
            break;
        case 'w':
            Editor.cursorY--;
            break;
        case 's':
            Editor.cursorY++;
            break;
    }

}

/**
 * Puts the terminal into raw mode to allow for reading input one character at a time
 * without any processing by the terminal. This function saves the original terminal
 * attributes so that they can be restored when the program exits.
 */
void enableRawMode() {

    // Call disableRawMode() when the program exits.
    atexit(disableRawMode);

    // Get the current terminal attributes.
    if (tcgetattr(STDIN_FILENO, &Editor.original_termios) == -1) die("tcgetattr error");

    // Make a copy of the terminal attributes.
    struct termios raw = Editor.original_termios;

    /* Turn off various flags to enable raw mode.
     * Local Flags : https://www.gnu.org/software/libc/manual/html_node/Local-Modes.html
     * Input Flags : https://www.gnu.org/software/libc/manual/html_node/Input-Modes.html
     * Output Flags : https://www.gnu.org/software/libc/manual/html_node/Output-Modes.html 
     * Turning off flags by reversing their bits.
     * To reverse a bit flag, bitwise AND it with the bitwise NOT of the flag.
     * This unsets the flag, leaving all other bits unchanged.
     * */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);

    // Set the terminal attributes to the modified values.
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr error");
}

/**
 * Restores the original terminal attributes to disable raw mode.
 */
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &Editor.original_termios) == -1)
        die("tcsetattr error");
}

/**
 * Initializes the editor by getting the screen size.
 */
void initEditor() {
    Editor.cursorX = Editor.cursorY = 0;

    if (getWindowSize(&Editor.screenRows, &Editor.screenCols) == -1)
        die("getWindowSize error");
}

/**
 * Gets the size of the terminal window in columns and rows.
 *
 * @param rows A pointer to an integer where the number of rows will be stored.
 * @param cols A pointer to an integer where the number of columns will be stored.
 * @return 0 on success, -1 on error.
 */
int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    // Use ioctl to get the size of the terminal window.
    // TIOCGWINSZ: Terminal Input/Output Control Get Window Size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
        return -1;
    else{
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/**
 * Prints an error message and exits the program.
 *
 * @param message The error message to print.
 */
void die(const char *message) {
    disableRawMode();
    perror(message); // Error message
    write(STDOUT_FILENO, "\x1b[2J", 4); // Clear screen
    exit(1);
}

#endif
