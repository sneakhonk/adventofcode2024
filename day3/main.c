#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAX_DIGITS 3

enum State {
    STATE_START,
    STATE_M,
    STATE_MU,
    STATE_MUL,
    STATE_LPAREN,
    STATE_X,
    STATE_COMMA,
    STATE_Y,
    STATE_RPAREN
};

int main() {
    int fd;
    struct stat st;
    size_t filesize;
    char *data;
    size_t offset = 0;
    uint64_t sum = 0;
    int state = STATE_START;
    char c;
    int x = 0;
    int y = 0;
    int x_digits = 0;
    int y_digits = 0;

    fd = open("input", O_RDONLY);
    if (fd == -1) {
        handle_error("open");
    }

    if (fstat(fd, &st) == -1) {
        close(fd);
        handle_error("stat");
    }

    filesize = st.st_size;
    data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        handle_error("mmap");
    }

    while ((c = data[offset]) != '\0') {
        switch (state) {
            case STATE_START:
                if (c == 'm') {
                    state = STATE_M;
                }
                break;
            case STATE_M:
                if (c == 'u') {
                    state = STATE_MU;
                } else {
                    state = STATE_START;
                    if (c == 'm') state = STATE_M;
                }
                break;
            case STATE_MU:
                if (c == 'l') {
                    state = STATE_MUL;
                } else {
                    state = STATE_START;
                    if (c == 'm') state = STATE_M;
                }
                break;
            case STATE_MUL:
                if (c == '(') {
                    state = STATE_LPAREN;
                    x = 0;
                    x_digits = 0;
                } else {
                    state = STATE_START;
                    if (c == 'm') state = STATE_M;
                }
                break;
            case STATE_LPAREN:
                if (isdigit(c)) {
                    x = x * 10 + (c - '0');
                    x_digits++;
                    if (x_digits > MAX_DIGITS) {
                        state = STATE_START;
                    } else {
                        state = STATE_X;
                    }
                } else {
                    state = STATE_START;
                }
                break;
            case STATE_X:
                if (isdigit(c)) {
                    x = x * 10 + (c - '0');
                    x_digits++;
                    if (x_digits > MAX_DIGITS) {
                        state = STATE_START;
                    }
                } else if (c == ',') {
                    state = STATE_COMMA;
                    y = 0;
                    y_digits = 0;
                } else {
                    state = STATE_START;
                }
                break;
            case STATE_COMMA:
                if (isdigit(c)) {
                    y = y * 10 + (c - '0');
                    y_digits++;
                    if (y_digits > MAX_DIGITS) {
                        state = STATE_START;
                    } else {
                        state = STATE_Y;
                    }
                } else {
                    state = STATE_START;
                }
                break;
            case STATE_Y:
                if (isdigit(c)) {
                    y = y * 10 + (c - '0');
                    y_digits++;
                    if (y_digits > MAX_DIGITS) {
                        state = STATE_START;
                    }
                } else if (c == ')') {
                    state = STATE_RPAREN;
                } else {
                    state = STATE_START;
                }
                break;
            case STATE_RPAREN:
                sum += x * y;
                state = STATE_START;
                if (c == 'm') state = STATE_M;
                break;
        }

        offset++;
    }

    printf("First star: %zu\n", sum);

    munmap(data, filesize);
    close(fd);
    return 0;
}
