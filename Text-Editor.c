/** Includes **/
#include "terminal_functions.h"
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

/** Macros **/
#define CTRL_KEY(c) ((c) & 0x1f) /* ands with 0x1f the given key (thats what ctrl key does) */

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

		if(c==CTRL_KEY('q')) break;
	}

	return 0;
}

