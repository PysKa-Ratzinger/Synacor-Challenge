#pragma once

#include "machine.h"
#include "data_structures/circular_array.h"

struct debugger;

struct debugger* machine_debugger_create();

void machine_debugger_free(struct debugger* d);

/*
 * machine_dump: Dumps debug information from the machine into the screen
 */
void machine_dump(struct machine* machine);

void machine_attach_debugger(struct machine* m, struct debugger* d);

void debugger_tick(struct debugger* d);

void debugger_set_breakpoint(struct debugger* d, uint16_t ip);

void debugger_unset_breakpoint(struct debugger* d, uint16_t ip);

void debugger_debug_enable(struct debugger* d);

void debugger_debug_disable(struct debugger* d);

void debugger_print_memory(struct debugger* d, uint16_t addr, uint16_t size);

void debugger_diff_memory(uint16_t* ram1, uint16_t* ram2, uint16_t addr,
		uint16_t size);

void debugger_save_state(struct debugger* d, size_t pos);

void debugger_load_state(struct debugger* d, size_t pos);

void debugger_save_stack(struct debugger* d, int save_pos);

void debugger_compare_stacks(struct debugger* d, int pos0, int pos1);

void debugger_save_memory(struct debugger* d, int save_pos);

void debugger_load_memory(struct debugger* d, int save_pos);

void debugger_compare_memory(struct debugger* d, int pos0, int pos1);

