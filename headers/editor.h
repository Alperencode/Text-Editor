#ifndef EDITOR_H
#define EDITOR_H

/** Macros **/
#define VERSION "0.0.1"
#define CTRL_KEY(c) ((c) & 0x1f) /* ands with 0x1f the given key (what ctrl key does) */

/** Enums **/
enum editorKeys{
    ARROW_LEFT = 'a',
    ARROW_RIGHT = 'd',
    ARROW_UP= 'w',
    ARROW_DOWN = 's',
}

/** Function Prototypes **/
char editorReadKey();
void editorProcessKeypress();
void editorDrawRows();
void editorRefreshScreen();
void editorMoveCursor(char);
void initEditor();
int getWindowSize(int*, int*);

void enableRawMode();
void disableRawMode();

void die(const char*);


/** Structs **/
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

    // If key is escape sequence
    if(c == '\x1b'){
        char seq[3];

        // If user does not enter any key after ESC, return ESC
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        // If its valid escape sequence
        if(seq[0] == '['){
            switch(seq[1]){
                // Matching arrow key movements
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
            }
        }

        return '\x1b';

    }else
        // Else, its normal char
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
    else if(c == ARROW_UP || ARROW_DOWN || ARROW_RIGHT || ARROW_LEFT)
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

    // Update cursor position with new values
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
        case ARROW_LEFT:
            Editor.cursorX--;
            break;
        case ARROW_RIGHT:
            Editor.cursorX++;
            break;
        case ARROW_UP:
            Editor.cursorY--;
            break;
        case ARROW_DOWN:
            Editor.cursorY++;
            break;
    }

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
    perror(message);
    write(STDOUT_FILENO, "\x1b[2J", 4); // Clear screen
    exit(1);
}

#endif
