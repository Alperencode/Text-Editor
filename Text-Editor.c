#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

void disableRawMode();
void enableRawMode();

struct termios o_termios;

int main(){
	char c;

	enableRawMode();

        /* reading 1 byte and assigning	it to 'c'
	 * read() returns the number of bytes that it read
	 * Terminal starts canonical mode (cooked mode)
	 * In canonical mode, it sends data to program when user hits enter */
	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
		// iscontrl test if char is a control char
		if(iscntrl(c))
			printf("%d\n", c);
		// else we are printing the char's ASCII code and the char
		else
			printf("%d('%c') ", c, c);
	}
	return 0;
}

void enableRawMode(){
	/* In raw mode it passed every char to program */

	// Calls disable function when program ends
	atexit(disableRawMode);

	// gathering terminal attributes
	tcgetattr(STDIN_FILENO, &o_termios);

	struct termios raw = o_termios;

	/* c_lflag: local flags
	 * ~ : bitwise not operator 
	 * ECHO is a bitflag and we disabling it by reversing it's bits
	 * We reversing it's bits with bitwise-AND operator
	 * ICANON is canonical mode flag and we are reversing it too
	 * */
	raw.c_lflag &= ~(ECHO | ICANON);

	/* applying terminal attributes
	 * TCAFLUS: specifies when to apply change */
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(){
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &o_termios);
}
