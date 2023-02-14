#include <unistd.h>

int main(){
	char c;

        /* reading 1 byte and assigning	it to 'c'
	 * read() returns the number of bytes that it read
	 * Terminal starts canonical mode (cooked mode)
	 * In canonical mode, it sends data to program when user hits enter */
	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q');

	return 0;
}
