#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_WIDTH 5
#define FIRST_NUM_OFFSET 0
#define SECOND_NUM_OFFSET 8
#define LINE_LEN 14
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int uint32_t_compare(const void *a, const void *b) {
    uint32_t arg1 = *(const uint32_t *)a;
    uint32_t arg2 = *(const uint32_t *)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

uint32_t get_number(const char* data) {
    char buffer[NUM_WIDTH + 1];
    memcpy(buffer, data, NUM_WIDTH);
    buffer[NUM_WIDTH] = '\0';
    char *endptr;
    errno = 0;
    unsigned long number = strtoul(buffer, &endptr, 10);
    if (endptr != buffer + NUM_WIDTH || errno != 0 || number > UINT32_MAX) {
        fprintf(stderr, "Invalid number: %.*s\n", NUM_WIDTH, buffer);
        return UINT32_MAX;
    }
    return (uint32_t)number;
}

size_t count_all_occurrences(uint32_t *sorted_list, size_t list_size, uint32_t key) {
    uint32_t *found = (uint32_t *)bsearch(&key, sorted_list, list_size, sizeof(uint32_t), uint32_t_compare);
    if (found == NULL) {
        return 0;
    }

    size_t index = found - sorted_list;
    size_t count = 1;

    for (ssize_t i = index - 1; i >= 0 && sorted_list[i] == key; --i) {
        count++;
    }

    for (size_t i = index + 1; i < list_size && sorted_list[i] == key; ++i) {
        count++;
    }

    return count;
}

int main() {
    int fd;
    struct stat st;
    size_t filesize;
    size_t listsize;
    char *data;
    uint32_t *first_list;
    uint32_t *second_list;
    size_t first_star;
    size_t second_star;

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

    if (filesize % LINE_LEN != 0) {
        munmap(data, filesize);
        close(fd);
        handle_error("filesize");
    }

    listsize = filesize / LINE_LEN;

    first_list = malloc(listsize * sizeof(uint32_t));
    second_list = malloc(listsize * sizeof(uint32_t));
    if (!first_list || !second_list) {
        munmap(data, filesize);
        close(fd);
        handle_error("malloc");
    }

    for (size_t idx = 0, offset = 0; idx < listsize; ++idx, offset += LINE_LEN) {
        first_list[idx] = get_number(data + offset + FIRST_NUM_OFFSET);
        second_list[idx] = get_number(data + offset + SECOND_NUM_OFFSET);
        if (first_list[idx] == UINT32_MAX || second_list[idx] == UINT32_MAX) {
            munmap(data, filesize);
            close(fd);
            free(first_list);
            free(second_list);
            handle_error("get_number");
        }
    }

    qsort(first_list, listsize, sizeof(uint32_t), uint32_t_compare);
    qsort(second_list, listsize, sizeof(uint32_t), uint32_t_compare);

    first_star = 0;
    second_star = 0;
    for (size_t i = 0; i < listsize; ++i) {
        second_star += count_all_occurrences(second_list, listsize, first_list[i]) * first_list[i];
        if (first_list[i] > second_list[i])
            first_star += (first_list[i] - second_list[i]);
        else
            first_star += (second_list[i] - first_list[i]);
    }

    printf("First star: %zu\n", first_star);
    printf("Second star: %zu\n", second_star);

    munmap(data, filesize);
    close(fd);
    free(first_list);
    free(second_list);
    return 0;
}
