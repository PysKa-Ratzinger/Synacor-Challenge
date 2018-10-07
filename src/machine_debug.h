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

