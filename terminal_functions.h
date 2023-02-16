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
char editorReadKey(){
	int readReturn;
	char c;
	while((readReturn = read(STDIN_FILENO, &c, 1)) != 1)
		if(readReturn == -1) die("read error");
	return c;
}

void editorProcessKeypress(){
	char c = editorReadKey();

	if(c==CTRL_KEY('q'))
		die("Exit Program");
}

void editorDrawRows(){
	int i;
	for(i=0; i<Editor.screenRows; i++)
		write(STDOUT_FILENO, "~\r\n", 3);
}

void editorRefreshScreen(){
	/*
	 * escape sequence: \x1b[
	 * \x1b: escape char and 27 in decimal
	 * escape sequence always starts with escape char followed by [
	 * 2: cleaning entire screen (1 for just up)
	 * J: erase in display command
	 * H: cursor position
	 * */
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3);
}

void enableRawMode(){
	/* In raw mode it passed every pressed char to program */

	// Calls disable function when program ends
	atexit(disableRawMode);

	// Gathering terminal attributes
	if(tcgetattr(STDIN_FILENO, &Editor.original_termios) == -1) die("tcgetattr error");

	struct termios raw = Editor.original_termios;

	/* Bitflags 
	 * ~ : bitwise not operator 
	 * Reversing given bitflags with bitwise-AND operator
	 * */

	/* c_lflag: Local Flags
	 * - ECHO for echo flag (echo of input characters back to the terminal)
	 * - ICANON for canonical mode 
	 * - ISIG for ctrl+c and ctrl+z signals
	 * - IEXTEN for ctrl+v signal (cannot rely on this interpretation on all systems)
	 * Local Flags explained here: https://www.gnu.org/software/libc/manual/html_node/Local-Modes.html
	 * */
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	/* c_iflag: Input Flags
	 * - IXON for ctrl+s and ctrl+q signal
	 * - ICRNL for ctrl+m and enter
	 * Input Flags explained here: https://www.gnu.org/software/libc/manual/html_node/Input-Modes.html 
	 * */
	raw.c_iflag &= ~(IXON | ICRNL);

	/* o_iflag: Output Flags
	 * - OPOST for output processing
	 * Output Flags explained here: https://www.gnu.org/software/libc/manual/html_node/Output-Modes.html 
	 * */
	raw.c_oflag &= ~(OPOST);

	/* Applying terminal attributes
	 * TCAFLUS: specifies when to apply change */
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr error");
}

void disableRawMode(){
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &Editor.original_termios) == -1)
		die("tcsetattr error");
}

void initEditor(){
	if(getWindowSize(&Editor.screenRows, &Editor.screenCols) == -1)
		die("getWindowSize error");
}

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;

	// TIOCGWINSZ: Terminal Input/Output Control Get Window Size
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
		return -1;
	else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

void die(const char *message){
	disableRawMode();
	editorRefreshScreen();
	perror(message);
	exit(1);
}
