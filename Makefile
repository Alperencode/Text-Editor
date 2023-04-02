Text-Editor: Text-Editor.c
	$(CC) Text-Editor.c editor.c append_buffer.c raw_mode.c -o Text-Editor -Wall -Wextra -pedantic -std=c99 && trap '$(RM) Text-Editor' EXIT; ./Text-Editor
	$(RM) Text-Editor
