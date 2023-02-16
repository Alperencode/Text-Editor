/** Terminal Functions **/

/** Includes **/
#include <termios.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

/** Function Prototypes **/
char editorReadKey();
void editorProcessKeypress();
void editorDrawRows();
void editorRefreshScreen();
void enableRawMode();
void disableRawMode();
void initEditor();
int getWindowSize(int*, int*);
void die(const char*);

/** Macros **/
#define CTRL_KEY(c) ((c) & 0x1f) /* ands with 0x1f the given key (what ctrl key does) */

/** Global Struct **/
struct editorConfig{
    int screenRows, screenCols;
    struct termios original_termios;
};

struct editorConfig Editor;

/** Functions **/

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
}

/**
 * Draws the rows of the editor to the screen.
 */
void editorDrawRows() {
    int i;

    // Write a tilde character to each row of the screen, followed by a newline character.
    for (i = 0; i < Editor.screenRows; i++)
        write(STDOUT_FILENO, "~\r\n", 3);
}


/**
 * Clears the screen and redraws the editor contents.
 */
void editorRefreshScreen() {

    // Send ANSI escape sequences to clear the screen and reset the cursor position.
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    // Redraw the editor rows.
    editorDrawRows();

    // Reset the cursor position to the top-left corner of the screen.
    write(STDOUT_FILENO, "\x1b[H", 3);
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
    editorRefreshScreen();
    perror(message);
    exit(1);
}
