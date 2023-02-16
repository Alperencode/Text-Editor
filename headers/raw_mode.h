#ifndef RAW_MODE_H
#define RAW_MODE_H

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

#endif