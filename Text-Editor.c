/** Includes **/
#include "terminal_functions.h"

/** Main **/
int main(){

	enableRawMode();

	while(1)
		editorProcessKeypress();

	return 0;
}
