#ifndef MACHINE_H_
#define MACHINE_H_

#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * struct machine: Represents the state of the virtual machine at any point
 * in time
 */
struct machine {
    /* ram: 15-bit address space storing 16-bit values
     */
    uint16_t ram[0x1<<15];

    /* reg: 8 registers holding 16-bit values
     */
    uint16_t reg[8];

    /*
     * stack: Unbounded stack which holds individual 16-bit values
     */
    struct stack16* stack;
};

/*
 * machine_new: Creates a new empty virtual machine
 */
struct machine* machine_new();

/*
 * machine_free: Frees a virtual machine
 */
void machine_free(struct machine* machine);

/*
 * machine_load_program: Loads a program into the machines memory
 *
 * Returns the size of the program in bytes
 */
size_t machine_load_program(struct machine* machine, int fd);

#endif  // MACHINE_H_

