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

#define WORD "XMAS"
#define WORD_LEN 4

#define CHECK_DIR(r, c, dr, dc) ({ \
    int found = 1; \
    for (int k = 0; k < WORD_LEN; k++) { \
        int rr = (r) + (dr)*k; \
        int cc = (c) + (dc)*k; \
        if (rr < 0 || rr >= rows || cc < 0 || cc >= cols || grid[rr][cc] != WORD[k]) { \
            found = 0; \
            break; \
        } \
    } \
    found; \
})

int main() {
    int fd;
    struct stat st;
    size_t filesize;
    char *data;

    fd = open("input", O_RDONLY);
    if (fd == -1) {
        perror("open"); 
        exit(EXIT_FAILURE); 
    }

    if (fstat(fd, &st) == -1) {
        close(fd);
        perror("stat");
        exit(EXIT_FAILURE); 
    }

    filesize = st.st_size;
    data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        perror("mmap");
        exit(EXIT_FAILURE); 
    }

    int rows = 0;
    int cols = -1;

    {
        int current_len = 0;
        for (size_t i = 0; i < filesize; i++) {
            if (data[i] == '\n') {
                rows++;
                if (cols == -1) {
                    cols = current_len;
                } else if (current_len != cols) {
                    munmap(data, filesize);
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                current_len = 0;
            } else {
                current_len++;
            }
        }
        if (cols == -1 && filesize > 0) {
            rows = 1;
            cols = (int)filesize;
        } else if (filesize > 0 && data[filesize-1] != '\n') {
            rows++;
            if (cols == -1) cols = current_len;
            else if (current_len != cols) {
                munmap(data, filesize);
                close(fd);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (rows == 0 || cols == 0) {
        munmap(data, filesize);
        close(fd);
        return 0;
    }

    char **grid = malloc(rows * sizeof(char *));
    if (!grid) {
        perror("malloc");
        munmap(data, filesize);
        close(fd);
        exit(EXIT_FAILURE);
    }

    {
        int r = 0;
        int c = 0;
        grid[r] = malloc(cols * sizeof(char));
        if (!grid[r]) {
            perror("malloc");
            munmap(data, filesize);
            close(fd);
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < filesize; i++) {
            if (data[i] == '\n') {
                r++;
                if (r < rows) {
                    grid[r] = malloc(cols * sizeof(char));
                    if (!grid[r]) {
                        perror("malloc");
                        for (int rr = 0; rr < r; rr++)
                            free(grid[rr]);
                        free(grid);
                        munmap(data, filesize);
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                }
                c = 0;
            } else {
                grid[r][c++] = data[i];
            }
        }
    }

    int found_count = 0;
    int directions[8][2] = {
        {0,1},   // right
        {0,-1},  // left
        {1,0},   // down
        {-1,0},  // up
        {1,1},   // down-right
        {1,-1},  // down-left
        {-1,1},  // up-right
        {-1,-1}  // up-left
    };

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == WORD[0]) {
                for (int d = 0; d < 8; d++) {
                    int dr = directions[d][0];
                    int dc = directions[d][1];
                    if (CHECK_DIR(r, c, dr, dc)) {
                        found_count++;
                    }
                }
            }
        }
    }

    printf("First star: %d\n", found_count);

    for (int r = 0; r < rows; r++) {
        free(grid[r]);
    }
    free(grid);

    munmap(data, filesize);
    close(fd);
    return 0;
}
