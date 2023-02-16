/** Includes **/
#include "terminal_functions.h"

/** Main **/
int main(){

	enableRawMode();
	initEditor();

	while(1){
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
