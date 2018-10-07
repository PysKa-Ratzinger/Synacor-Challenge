#pragma once

#include "machine.h"
#include "circular_array.h"

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

void debugger_print_memory(struct debugger* d, uint16_t start, uint16_t offset);

void debugger_save_state(struct debugger* d, size_t pos);

void debugger_load_state(struct debugger* d, size_t pos);

