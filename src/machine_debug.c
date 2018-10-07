#include "machine_debug.h"
#include "common.h"
#include "stack.h"

#include <stdio.h>

#define CIRCULAR_SIZE 105
#define MAX_BREAKPOINTS 500
#define MAX_STATES 10

struct debugger {
	struct machine* m;
	struct circular_array_16* ip_history;

	struct machine* states[MAX_STATES];
	char states_exist[MAX_STATES];

	uint16_t breakpoints[500];
	size_t num_breakpoints;

	size_t debug_opcodes;
	size_t skips;
	char debug_ip_history;
	char debug_stack;
	char debug_regs;
	char debug_disass;
	char debug_enabled;
	char debug_memory;
};

struct debugger* machine_debugger_create() {
	struct debugger* res = malloc(sizeof(*res));
	memset(res, 0, sizeof(*res));
	res->ip_history = circular_array_create(CIRCULAR_SIZE);
	res->num_breakpoints = 0;
	res->debug_ip_history = 1;
	res->debug_disass = 1;
	res->debug_opcodes = 15;
	res->debug_regs = 1;
	res->debug_enabled = 0;
	return res;
}

void machine_debugger_free(struct debugger* d) {
	free(d);
}

void debugger_debug_enable(struct debugger* d) {
	d->debug_enabled = 1;
}

void debugger_debug_disable(struct debugger* d) {
	d->debug_enabled = 0;
}

static void debugger_print_ip_history(struct debugger* dbg) {
	printf("HISTORY BEGIN:\n");
	circular_array_print(dbg->ip_history);
	printf("HISTORY END\n");
}

static void debugger_print_stack(struct machine* m) {
	stack16_print(m->stack);
}

static char* machine_get_mem_repr(uint16_t addr) {
	static char res[50];
	if (addr <= 0x7fff) {
		snprintf(res, 50, "%04x", addr);
		return res;
	}

	addr &= 0x7fff;
	if (addr <= 7) {
		snprintf(res, 50, "R%u", addr);
	} else {
		snprintf(res, 50, "%04x?", addr);
	}
	return res;
}

static void machine_print_regs(struct machine* machine) {
	uint16_t *reg = machine->reg;
	printf("R0: %04x, R1: %04x, R2: %04x, R3: %04x\n",
			reg[0], reg[1], reg[2], reg[3]);
	printf("R4: %04x, R5: %04x, R6: %04x, R7: %04x\n",
			reg[4], reg[5], reg[6], reg[7]);
	printf("IP: %04x\n", (uint16_t)(machine->ip - machine->ram));
}

static int debugger_disas(struct machine* machine, size_t opcodes) {
	uint16_t *fake_ip = machine->ip;
	uint16_t op;

	char *op_names[] = {
		"HALT", "SET", "PUSH", "POP", "EQ",  "GT", "JMP", "JNZ",
		"JZ",   "ADD", "MULT", "MOD", "AND", "OR", "NOT", "RMEM",
		"WMEM", "CALL", "RET", "OUT", "IN", "NOP"
	};

	uint16_t op_size[] = {
		0, 2, 1, 1, 3, 3, 1, 2,
		2, 3, 3, 3, 3, 3, 2, 2,
		2, 1, 0, 1, 1, 0
	};

	for (; opcodes; opcodes--) {
		op = *fake_ip;
		printf("0x%04lx: ", fake_ip - machine->ram);
		if (op >= ARRAY_SIZE(op_names)) {
			printf("%04x%19s???\n", op, "");
		} else {
			size_t num_ops = op_size[op];
			printf("%-4s", op_names[op]);
			for (size_t i=0; i<num_ops; i++) {
				printf(" %-5s", machine_get_mem_repr(fake_ip[1+i]));
			}
			printf("\n");
			fake_ip += num_ops;
		}
		fake_ip++;
	}
	return 0;
}

void machine_dump(struct machine* machine) {
	struct debugger* dbg = machine->debugger;
	if (dbg == NULL)
		return;

	printf("=========== DEBUG INFO ==============\n");
	if (dbg->debug_ip_history) {
		debugger_print_ip_history(dbg);
		printf("-------------------------------------\n");
	}

	if (dbg->debug_memory) {
		debugger_print_memory(dbg, 0, 0);
		printf("-------------------------------------\n");
	}

	if (dbg->debug_stack) {
		debugger_print_stack(machine);
		printf("-------------------------------------\n");
	}

	if (dbg->debug_regs) {
		machine_print_regs(machine);
		printf("-------------------------------------\n");
	}

	if (dbg->debug_disass) {
		debugger_disas(machine, dbg->debug_opcodes);
		printf("-------------------------------------\n");
	}
	printf("=========== DEBUG END ===============\n");
}

void machine_attach_debugger(struct machine* m, struct debugger* d) {
	m->debugger = d;
	d->m = m;
}

static void debugger_list_breakpoints(struct debugger* d) {
	printf("BREAKPOINTS: \n");
	if (d->num_breakpoints == 0) {
		printf("   EMPTY\n");
		return;
	}
	for (size_t i=0; i<d->num_breakpoints; i++) {
		printf(" + %04x\n", d->breakpoints[i]);
	}
}

static int debugger_shell(struct debugger* d) {
	static char prev_buffer[256];

	printf("(debug) ");
	fflush(stdout);
	char buffer[256];
	fgets(buffer, sizeof(buffer), stdin);

	char *cmd = buffer;

	if (buffer[0] == '\n') {
		cmd = prev_buffer;
	} else {
		memcpy(prev_buffer, buffer, sizeof(256));
	}

	if (strncmp(cmd, "history_on", 10) == 0) {
		d->debug_ip_history = 1;
	} else if (strncmp(cmd, "history_off", 11) == 0) {
		d->debug_ip_history = 0;
	} else if (strncmp(cmd, "stack_on", 8) == 0) {
		d->debug_stack = 1;
	} else if (strncmp(cmd, "stack_off", 9) == 0) {
		d->debug_stack = 0;
	} else if (strncmp(cmd, "regs_on", 7) == 0) {
		d->debug_regs = 1;
	} else if (strncmp(cmd, "regs_off", 8) == 0) {
		d->debug_regs = 0;
	} else if (strncmp(cmd, "disass_on", 9) == 0) {
		d->debug_disass = 1;
	} else if (strncmp(cmd, "disass_off", 10) == 0) {
		d->debug_disass = 0;
	} else if (strncmp(cmd, "memory_off", 10) == 0) {
		d->debug_memory = 0;
	} else if (strncmp(cmd, "dops", 4) == 0) {
		d->debug_opcodes = strtol(cmd + 5, NULL, 16);
	} else if (strncmp(cmd, "save", 4) == 0) {
		size_t pos = strtol(cmd+5, NULL, 10);
		debugger_save_state(d, pos);
	} else if (strncmp(cmd, "load", 4) == 0) {
		size_t pos = strtol(cmd+5, NULL, 10);
		debugger_load_state(d, pos);
	} else if (strncmp(cmd, "s", 1) == 0) {
		return 1;
	} else if (strncmp(cmd, "b", 1) == 0) {
		debugger_set_breakpoint(d, strtol(cmd+2, NULL, 16));
	} else if (strncmp(cmd, "ub", 2) == 0) {
		debugger_unset_breakpoint(d, strtol(cmd+3, NULL, 16));
	} else if (strncmp(cmd, "lb", 2) == 0) {
		debugger_list_breakpoints(d);
	} else if (strncmp(cmd, "c", 1) == 0) {
		debugger_debug_disable(d);
		d->skips = strtol(cmd+2, NULL, 16);
		return 1;
	} else if (strncmp(cmd, "p", 1) == 0) {
		char* endptr;
		uint16_t start_pos = strtol(cmd+2, &endptr, 16);
		uint16_t offset = strtol(endptr, NULL, 16);
		debugger_print_memory(d, start_pos, offset);
	}

	return 0;
}

static int debugger_get_breakpoint(struct debugger* d, uint16_t ip) {
	if (d->num_breakpoints == 0)
		return -1;
	for (size_t i=0; i<d->num_breakpoints; i++) {
		if (d->breakpoints[i] == ip) {
			return i;
		}
	}
	return -1;
}

void debugger_set_breakpoint(struct debugger* d, uint16_t ip) {
	if (d->num_breakpoints + 1 >= MAX_BREAKPOINTS) {
		printf("ERROR! Max breakpoints reached...\n");
		return;
	}
	if (debugger_get_breakpoint(d, ip) != -1)
		return;
	d->breakpoints[d->num_breakpoints] = ip;
	d->num_breakpoints++;
}

void debugger_unset_breakpoint(struct debugger* d, uint16_t ip) {
	if (d->num_breakpoints == 0)
		return;
	int index = debugger_get_breakpoint(d, ip);
	if (index == -1)
		return;
	d->breakpoints[index] = d->breakpoints[d->num_breakpoints-1];
	d->num_breakpoints--;
}

void debugger_tick(struct debugger* d) {
	uint16_t ip = d->m->ip - d->m->ram;
	circular_array_insert(d->ip_history, ip);

	if (!d->debug_enabled) {
		if (debugger_get_breakpoint(d, ip) != -1) {
			if (d->skips == 0) {
				debugger_debug_enable(d);
			} else {
				d->skips--;
			}
		}
	}

	if (d->debug_enabled) {
		do {
			printf("\n");
			fflush(stdout);
			machine_dump(d->m);
		} while (debugger_shell(d) == 0);
	}

	return;
}

char get_printable(uint16_t val) {
	if (val >= 0x21 && val <= 0x7e) {
		return (char) val;
	}
	return '.';
}

void debugger_print_memory(struct debugger* d, uint16_t start, uint16_t offset) {
	static uint16_t real_start = 0;
	static uint16_t real_offset = 0;
	if (start != offset) {
		real_start = start;
		real_offset = offset;
	} else {
		start = real_start;
		offset = real_offset;
	}
	d->debug_memory = 1;


	start &= 0x7ff0;

	printf("MEMORY DUMP (%04x, %04x)\n", start, start + offset);
	while (offset) {
		uint16_t* ram = d->m->ram;
		printf("%04x: %04x %04x %04x %04x  "
				"%04x %04x %04x %04x    "
				"%04x %04x %04x %04x  "
				"%04x %04x %04x %04x | %c%c%c%c%c%c%c%c "
				"%c%c%c%c%c%c%c%c |\n", start,
				ram[start],
				ram[start+1],
				ram[start+2],
				ram[start+3],
				ram[start+4],
				ram[start+5],
				ram[start+6],
				ram[start+7],
				ram[start+8],
				ram[start+9],
				ram[start+10],
				ram[start+11],
				ram[start+12],
				ram[start+13],
				ram[start+14],
				ram[start+15],
				get_printable(ram[start]),
				get_printable(ram[start+1]),
				get_printable(ram[start+2]),
				get_printable(ram[start+3]),
				get_printable(ram[start+4]),
				get_printable(ram[start+5]),
				get_printable(ram[start+6]),
				get_printable(ram[start+7]),
				get_printable(ram[start+8]),
				get_printable(ram[start+9]),
				get_printable(ram[start+10]),
				get_printable(ram[start+11]),
				get_printable(ram[start+12]),
				get_printable(ram[start+13]),
				get_printable(ram[start+14]),
				get_printable(ram[start+15]));
		start += 16;
		if (offset <= 16)
			break;
		offset -= 16;
	}
	printf("\n");
}

void debugger_save_state(struct debugger* d, size_t pos) {
	if (pos > MAX_STATES)
		return;
	if (d->states[pos]) {
		machine_free(d->states[pos]);
	}
	d->states[pos] = machine_duplicate(d->m);
	d->states_exist[pos] = 1;
	printf("Saved into state %lu\n", pos);
}

void debugger_load_state(struct debugger* d, size_t pos) {
	if (pos > MAX_STATES)
		return;
	if (d->states_exist[pos]) {
		d->m->debugger = NULL;
		machine_free(d->m);
		d->m = machine_duplicate(d->states[pos]);
		d->m->debugger = d;
	}
	printf("Loaded state %lu\n", pos);
}

