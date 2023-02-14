Text-Editor: Text-Editor.c
	$(CC) Text-Editor.c -o Text-Editor -Wall -Wextra -pedantic -std=c99 && trap '$(RM) Text-Editor' EXIT; ./Text-Editor
	$(RM) Text-Editor
