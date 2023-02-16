/** Includes **/
#include "editor.h"

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
