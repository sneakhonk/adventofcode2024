#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

size_t offset_incr(uint8_t num) {
    return (size_t)(num > 9 ? 2 : 1);
}

uint8_t scan_number(const char* data, size_t offset) {
    if (data[offset + 1] >= '0' && data[offset + 1] <= '9') {
        return (uint8_t)((data[offset] - '0') * 10 + (data[offset + 1] - '0'));
    }
    return (uint8_t)(data[offset] - '0');
}

uint8_t scan_with_dampener(const char* data, size_t const offset, size_t count) {
    uint8_t nums[256];
    size_t num_read = 0;
    size_t currentoffset = offset;

    while (num_read < count) {
        nums[num_read] = scan_number(data, currentoffset);
        currentoffset += offset_incr(nums[num_read]) + 1;
        num_read++;
    }

    for (size_t i = 0; i < count; ++i) {
        uint8_t lastnum = 0, nextnum = 0;
        int safe = 0;
        int first = 1;

        for (size_t j = 0; j < count; ++j) {
            if (j == i) continue;

            if (first) {
                lastnum = nums[j];
                first = 0;
                continue;
            }

            nextnum = nums[j];

            if (safe == 0) {
                if (lastnum > nextnum && ((lastnum - nextnum) <= 3)) safe = -1;
                else if (nextnum > lastnum && ((nextnum - lastnum) <= 3)) safe = 1;
                else {
                    safe = 0;
                    break;
                }
            } else {
                if ((safe == -1) && lastnum > nextnum && ((lastnum - nextnum) <= 3)) {
                } else if ((safe == 1) && nextnum > lastnum && ((nextnum - lastnum) <= 3)) {
                } else {
                    safe = 0;
                    break;
                }
            }
            lastnum = nextnum;
        }

        if (safe != 0) {
            return 1;
        }
    }

    return 0;
}


int main() {
    int fd;
    struct stat st;
    size_t filesize;
    char *data;
    size_t offset;
    int safe;
    uint8_t lastnum;
    uint8_t nextnum;
    size_t safe_count;
    size_t dampener_count;
    size_t last_report_offset;
    size_t last_report_length;

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
    
    offset = 0;
    last_report_offset = 0;
    last_report_length = 2;

    lastnum = scan_number(data, offset);
    offset += offset_incr(lastnum) + 1;
    nextnum = scan_number(data, offset);
    offset += offset_incr(nextnum);

    if (lastnum > nextnum && ((lastnum - nextnum) <= 3)) safe = -1;
    else if (nextnum > lastnum && ((nextnum - lastnum) <= 3)) safe = 1;
    else safe = 0;

    safe_count = 0;
    dampener_count = 0;

    while (offset < filesize) {

        if (data[offset] == '\n') {
            ++offset;
            if (safe != 0) 
            {
                ++safe_count;
            }
            else if (scan_with_dampener(data, last_report_offset, last_report_length) != 0) ++dampener_count;
            last_report_offset = offset;
            last_report_length = 2;
            lastnum = scan_number(data, offset);
            offset += offset_incr(lastnum) + 1;
            nextnum = scan_number(data, offset);
            offset += offset_incr(nextnum);

            if (lastnum > nextnum && ((lastnum - nextnum) <= 3)) safe = -1;
            else if (nextnum > lastnum && ((nextnum - lastnum) <= 3)) safe = 1;
            else safe = 0;
        }
        
        if (data[offset] == ' ') 
        {
            ++offset;
        }

        lastnum = nextnum;
        nextnum = scan_number(data, offset);
        ++last_report_length;
        offset += offset_incr(nextnum);

        if ((safe == -1) && lastnum > nextnum && ((lastnum - nextnum) <= 3)) continue;
        else if ((safe == 1) && nextnum > lastnum && ((nextnum - lastnum) <= 3)) continue;

        safe = 0;
    }

    printf("First star: %zu\n", safe_count);
    printf("Second star: %zu\n", safe_count + dampener_count);

    munmap(data, filesize);
    close(fd);
    return 0;
}
