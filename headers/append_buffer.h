#ifndef APPEND_BUFFER_H
#define APPEND_BUFFER_H

#define ABUF_INIT {NULL, 0}

struct appendBuffer{
    char *buffer;
    int len;
};

/**
 * Appends a string to an appendBuffer.
 *
 * @param ab A pointer to the append buffer to append the string to.
 * @param str A pointer to the string to append.
 * @param len The length of the string to append.
 */
void abAppend(struct appendBuffer *ab, const char *str, int len){
    char *new = realloc(ab->buffer, ab->len + len);
    
    if(new == NULL) return;

    // Copy the str after the end of current data in buffer
    memcpy(&new[ab->len], str, len);

    // Update the buffer with new values
    ab->buffer = new;
    ab->len += len;

}

/**
 * Frees the given appendBuffer.
 *
 * @param ab A pointer to the appendBuffer to free.
 */
void abFree(struct appendBuffer *ab){
    free(ab->buffer);
}

#endif
