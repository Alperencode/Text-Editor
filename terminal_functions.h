/** Terminal Functions **/

/** Includes **/
#include <termios.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/** Function Prototypes **/
char editorReadKey();
void editorProcessKeypress();
void enableRawMode();
void disableRawMode();
void die(const char*);

/** Macros **/
#define CTRL_KEY(c) ((c) & 0x1f) /* ands with 0x1f the given key (what ctrl key does) */

/** Global Variables **/
struct termios o_termios;

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

	if(c==CTRL_KEY('q')) die("Exit Program");
}

void enableRawMode(){
	/* In raw mode it passed every pressed char to program */

	// Calls disable function when program ends
	atexit(disableRawMode);

	// Gathering terminal attributes
	if(tcgetattr(STDIN_FILENO, &o_termios) == -1) die("tcgetattr error");

	struct termios raw = o_termios;

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
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &o_termios) == -1)
		die("tcsetattr error");
}

void die(const char *message){
	disableRawMode();
	perror(message);
	exit(1);
}
