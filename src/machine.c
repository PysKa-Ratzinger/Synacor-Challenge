#include "machine.h"

#include <unistd.h>

struct machine* machine_new() {
    struct machine* res = NULL;
    res = (struct machine*) malloc(sizeof(struct machine));
    if (res != NULL) {
        memset(res, 0, sizeof(struct machine));
    }
    return res;
}

void machine_free(struct machine* machine) {
    return free(machine);
}

size_t machine_load_program(struct machine* machine, int fd) {
    if (machine == NULL)
        return 0;
    ssize_t bytes_read;
    size_t total_bytes = 0;
    size_t buffer_left = sizeof(machine->ram);
    while ((bytes_read = read(fd, &machine->ram[total_bytes], buffer_left))) {
        total_bytes += bytes_read;
        buffer_left -= bytes_read;
        if (buffer_left == 0)
            break;
    }
    return total_bytes;
}

