#pragma once

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
    /*
     * ram: 15-bit address space storing 16-bit values
     */
    uint16_t ram[0x1<<15];

    /*
     * reg: 8 registers holding 16-bit values
     */
    uint16_t reg[8];

    /*
     * ip: Instruction pointer
     */
    uint16_t *ip;

    /*
     * stack: Unbounded stack which holds individual 16-bit values
     */
    struct stack16* stack;

    /*
     * debugger:09
     */
    struct debugger* debugger;
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
 * machine_tick: Executes the current machine code
 */
int machine_tick(struct machine* machine);

/*
 * machine_run: Runs the machine... :P
 */
void machine_run(struct machine* machine);

/*
 * machine_load_program: Loads a program into the machines memory
 *
 * Returns the size of the program in bytes
 */
size_t machine_load_program(struct machine* machine, int fd);

