#include <stdio.h>

#include "stack.h"

int main() {
    struct stack16* stack = stack16_create();
    for (uint16_t i=0; i<20; i++) {
        stack16_push(stack, i);
    }
    for (uint16_t i=0; i<22; i++) {
        uint16_t val;
        int e = stack16_pop(stack, &val);
        if (e == 0) {
            printf(" - %d\n", val);
        } else {
            break;
        }
    }
    stack16_free(stack);
    return 0;
}

