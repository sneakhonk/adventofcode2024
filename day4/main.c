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

static int rows, cols;
static char **grid;

static int check_word(int r, int c, int dr, int dc) {
    for (int k = 0; k < WORD_LEN; k++) {
        int rr = r + dr * k;
        int cc = c + dc * k;
        if (rr < 0 || rr >= rows || cc < 0 || cc >= cols || grid[rr][cc] != WORD[k]) {
            return 0;
        }
    }
    return 1;
}

static int check_x_pattern(int r, int c) {
    // The cell must be 'A'
    if (grid[r][c] != 'A') {
        return 0;
    }

    // Check that diagonals exist
    if (r - 1 < 0 || r + 1 >= rows || c - 1 < 0 || c + 1 >= cols) {
        return 0;
    }

    // Diagonal cells:
    char tl = grid[r-1][c-1]; // top-left
    char tr = grid[r-1][c+1]; // top-right
    char bl = grid[r+1][c-1]; // bottom-left
    char br = grid[r+1][c+1]; // bottom-right

    // We need one diagonal pair to contain 'M' and 'S' in any order:
    // Check first diagonal pair (top-left and bottom-right)
    int diag1 = ((tl == 'M' && br == 'S') || (tl == 'S' && br == 'M'));

    // Check second diagonal pair (top-right and bottom-left)
    int diag2 = ((tr == 'M' && bl == 'S') || (tr == 'S' && bl == 'M'));

    return (diag1 && diag2) ? 1 : 0;
}

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

    // Determine rows and cols
    {
        int current_len = 0;
        cols = -1;
        rows = 0;
        for (size_t i = 0; i < filesize; i++) {
            if (data[i] == '\n') {
                rows++;
                if (cols == -1) {
                    cols = current_len;
                } else if (current_len != cols) {
                    munmap(data, filesize);
                    close(fd);
                    fprintf(stderr, "Inconsistent column length\n");
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
                fprintf(stderr, "Inconsistent column length at end\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (rows == 0 || cols == 0) {
        munmap(data, filesize);
        close(fd);
        return 0;
    }

    grid = malloc(rows * sizeof(char *));
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
    int x_count = 0;

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

    // Find occurrences of WORD
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == WORD[0]) {
                for (int d = 0; d < 8; d++) {
                    int dr = directions[d][0];
                    int dc = directions[d][1];
                    if (check_word(r, c, dr, dc)) {
                        found_count++;
                    }
                }
            }
        }
    }

    // M-S
    // -A-
    // M-S
    // Find "X" patterns with 'A' in center and diagonals being 'M' and 'S'
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (check_x_pattern(r, c)) {
                x_count++;
            }
        }
    }

    printf("First star: %d\n", found_count);
    printf("Second star: %d\n", x_count);

    for (int r = 0; r < rows; r++) {
        free(grid[r]);
    }
    free(grid);

    munmap(data, filesize);
    close(fd);
    return 0;
}
