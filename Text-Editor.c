/** Includes **/
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

/** Functions Prototypes **/
void disableRawMode();
void enableRawMode();
void die(const char*);

/** Global Variables **/
struct termios o_termios;

/** Main **/
int main(){
	enableRawMode();
	char c;

	while(1){

        	/* Reading 1 byte and assigning it to 'c'
	 	* read() returns the number of bytes that it read
	 	* Terminal starts canonical mode (cooked mode)
	 	* In canonical mode, it sends data to program when user hits enter */
		if(read(STDIN_FILENO, &c, 1) == -1) die("read error");

		// iscontrl test if char is a control char, if so printing its ASCII
		if(iscntrl(c))
			printf("%d\r\n", c);
		// else printing the char's ASCII and the char
		else
			printf("%d('%c')\r\n", c, c);

		if(c=='q') break;
	}

	return 0;
}

/** Terminal Functions **/

void enableRawMode(){
	/* In raw mode it passed every char to program */

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
	perror(message);
	exit(1);
}
