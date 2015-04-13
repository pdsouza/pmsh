#include <stdio.h>
#include "history.h"

static const char hfile[] = ".pmsh_history";

static char * reverse_fgets(char *buf, int size, FILE *file) {
    char c;
    char *ret = NULL;
    long offset = -2L;
    long start, pos;

    start = pos = ftell(file);

    /*
        The history file looks like this:

        cmd_0\n
        cmd_1\n
        ...
        cmd_n\nEOF

        We successively read chars in reverse from the the
        initial SEEK_CUR until we hit a fresh newline.

        *All of the read calls move the current file pointer
        forward so we use an offset of -2.
    */
    while (ftell(file) > 0) {
        //printf("cur = %ld\n", pos);
        //printf("pre ftell = %li\n", ftell(file));
        c = fgetc(file);
        //printf("post ftell = %li\n", ftell(file));

        // if (c == EOF) {
        //     printf("fgetc(history) = EOF\n");
        // } else if (c == '\n') {
        //     printf("fgetc(history) = \\n\n");
        // } else {
        //     printf("fgetc(history) = %c\n", c);
        // }

        pos = ftell(file);

        // if we find a newline and it's not the
        // starting line's newline...we found the
        // start of the previous line!
        if (c == '\n' && pos < start) {
            ret = fgets(buf, size, file);
            fseek(file, pos - 1, SEEK_SET);
            break;
        }

        fseek(file, offset, SEEK_CUR);
        pos -= 1;
    }

    return ret;
}

int add_history(const char *cmd) {
    int ret;
    FILE *file = fopen(hfile, "a+");

    ret = fprintf(file, "%s\n", cmd);

    fclose(file);
    return ret;
}

const char* get_history(char *buf, int size, int idx) {
    int i;
    char *ret;
    FILE *file = fopen(hfile, "r");

    // we first seek to the end of the file (sans EOF)
    // to start reading from the top of the stack
    fseek(file, -1L, SEEK_END);

    while(idx-- >= 0) {
        ret = reverse_fgets(buf, size, file);
        printf("[DEBUG :: get_history] buf = %s", buf);
    }
    fclose(file);

    return ret;
}
