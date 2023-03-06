#include "editor.h"

// Version
#define VERSION "0.0.1"

// Bitwise and with 0x1f the given key (ctrl key function)
#define CTRL_KEY(c) ((c) & 0x1f)

// Reads y bytes and assigns it to x's address
#define READ(x, y) (read(STDIN_FILENO, &x, y) == -1)

/**
 * Waits for the user to press a key and returns the key that was pressed.
 *
 * @return The key that was pressed.
 */
int editorReadKey() {
    char c;

    // Wait for a single keypress from the user.
    if(READ(c,1)) die("read error");

    // If key is escape sequence
    if(c == '\x1b'){
        char seq[3];

        // Read 2 more bytes after sequence (with error handling)
        if (READ(seq[0],1)) return '\x1b';
        if (READ(seq[1],1)) return '\x1b';

        // If its valid escape sequence
        if(seq[0] == '['){

            // If its digit (command key)
            if(seq[1] >= '0' && seq[1] <= '9'){

                // Read one more byte 
                if (READ(seq[2],1)) return '\x1b';

                if (seq[2] == '~') {
                    switch(seq[1]){
                        // Matching key
                        case '1': return HOME_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            }
            else{
                switch(seq[1]){
                    // Matching key
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }

        }else if(seq[0] == 'O'){
            switch(seq[1]){
                // Matching key
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        // If its not any special key, return
        return '\x1b';

    }else
        // Else, its normal char
        return c;

}

/**
 * Processes a single keypress from the user and performs the appropriate action.
 */
void editorProcessKeypress() {
    int c = editorReadKey();

    switch(c) {
        // If the user pressed Ctrl-Q, exit the program.
        case CTRL_KEY('q'):
            die("Exit Program");
            break;

        // Arrow keys: Call MoveCursor
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_RIGHT:
        case ARROW_LEFT:
            editorMoveCursor(c);
            break;

        // Page up/down keys: Call MoveCursor in loop
        case PAGE_UP:
        case PAGE_DOWN:
            for(int i = 0; i < Editor.screenRows; i++)
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            break;

        // Default: pass
        default:
            break;
    }


}

/**
 * Draws the rows of the editor to the screen.
 */
void editorDrawRows(struct appendBuffer *ab) {

    // Write a tilde character to each row of the screen, followed by a newline character.
    for(int i = 0; i < Editor.screenRows; i++){

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

    // Creating empty buffer
    struct appendBuffer ab = ABUF_INIT;

    // Send escape sequences to hide cursor and reset the cursor position.
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

    // Sending buffer
    write(STDOUT_FILENO, ab.buffer, ab.len);

    // Clearing the buffer from memory
    abFree(&ab);
}

/**
 * Moves the cursor position in response to a user input key.
 *
 * @param key The key that was pressed to trigger the cursor movement.
 */
void editorMoveCursor(int key) {

    switch(key) {
        case ARROW_LEFT:
            if(Editor.cursorX != 0)
                Editor.cursorX--;
            break;
        case ARROW_RIGHT:
            if(Editor.cursorX != Editor.screenCols)
                Editor.cursorX++;
            break;
        case ARROW_UP:
            if(Editor.cursorY != 0)
                Editor.cursorY--;
            break;
        case ARROW_DOWN:
            if(Editor.cursorY != Editor.screenRows)
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
    clearScreen();
    perror(message);
    exit(1);
}

/**
 * Clears the terminal screen and resets cursor position
 */
void clearScreen(){
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}
