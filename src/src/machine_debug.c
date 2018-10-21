#include "machine_debug.h"
#include "common.h"
#include "data_structures/stack.h"
#include "machine.h"

#include <stdio.h>

#define CIRCULAR_SIZE 105
#define MAX_BREAKPOINTS 500
#define MAX_STATES 10
#define MAX_STACKS 10
#define MAX_MEMORIES 10

#define MAX_ADDR 0x7fff

struct debugger {
	struct machine* m;
	struct circular_array_16* ip_history;

	struct machine* states[MAX_STATES];
	struct stack16* stacks[MAX_STACKS];
	char states_exist[MAX_STATES];
	uint16_t *rams[MAX_MEMORIES];

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

static int debugger_disas(struct machine* machine, uint16_t* ip,
		size_t opcodes) {
	uint16_t *fake_ip = machine->ip;
	if (ip) {
		fake_ip = ip;
	}
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
		if ((size_t)(fake_ip - machine->ram) > MAX_ADDR)
			break;
		op = *fake_ip;
		printf("0x%04lx: ", fake_ip - machine->ram);
		if (op >= ARRAY_SIZE(op_names)) {
			printf("%04x%10s???\n", op, "");
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
		debugger_disas(machine, machine->ip, dbg->debug_opcodes);
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

	} else if (strncmp(cmd, "stack_save", 10) == 0) {
		int save_pos = strtol(cmd + 11, NULL, 10);
		debugger_save_stack(d, save_pos);

	} else if (strncmp(cmd, "stack_compare", 13) == 0) {
		char *endstr = NULL;
		int pos0 = strtol(cmd + 14, &endstr, 10);
		int pos1 = strtol(endstr, NULL, 10);
		debugger_compare_stacks(d, pos0, pos1);

	} else if (strncmp(cmd, "regs_on", 7) == 0) {
		d->debug_regs = 1;

	} else if (strncmp(cmd, "regs_off", 8) == 0) {
		d->debug_regs = 0;

	} else if (strncmp(cmd, "disass_on", 9) == 0) {
		d->debug_disass = 1;

	} else if (strncmp(cmd, "disass_off", 10) == 0) {
		d->debug_disass = 0;

	} else if (strncmp(cmd, "memory_on", 9) == 0) {
		d->debug_memory = 1;

	} else if (strncmp(cmd, "memory_off", 10) == 0) {
		d->debug_memory = 0;

	} else if (strncmp(cmd, "memory_save", 11) == 0) {
		int save_pos = strtol(cmd + 12, NULL, 10);
		debugger_save_memory(d, save_pos);

	} else if (strncmp(cmd, "memory_load", 11) == 0) {
		int save_pos = strtol(cmd + 12, NULL, 10);
		debugger_load_memory(d, save_pos);

	} else if (strncmp(cmd, "memory_cmp", 10) == 0) {
		char *endstr = NULL;
		int pos0 = strtol(cmd + 11, &endstr, 10);
		int pos1 = strtol(endstr, NULL, 10);
		debugger_compare_memory(d, pos0, pos1);

	} else if (strncmp(cmd, "dump", 4) == 0) {
		char *endstr = NULL;
		size_t addr = strtol(cmd+5, &endstr, 16);
		size_t opcodes = strtol(endstr, NULL, 16);
		debugger_disas(d->m, d->m->ram + addr, opcodes);

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

	} else {
		machine_dump(d->m);
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
		machine_dump(d->m);
		do {
			fflush(stdout);
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

void debugger_print_helper(uint16_t page, uint16_t* values,
		uint16_t* exist) {
	printf("%04x: ", page);
	for (int i=0; i<16; i++) {
		if (i == 8) {
			printf(" ");
		}
		if (exist[i]) {
			printf("%04x ", values[i]);
		} else {
			printf("     ");
		}
	}
	printf("| ");
	for (int i=0; i<16; i++) {
		if (i == 8) {
			printf(" ");
		}
		printf("%c", exist[i] ? get_printable(values[i]) : ' ');
	}
	printf(" |\n");
	fflush(stdout);
}

void debugger_print_memory(struct debugger* d, uint16_t addr, uint16_t size) {
	uint16_t buffer[16] = {0};
	uint16_t exist[16] = {0};
	uint16_t curr_page = addr & (~0xf);
	uint16_t next_page = curr_page + 0x10;
	size_t num_elems = 0;
	uint16_t* ram = d->m->ram;

	printf("MEMORY DUMP (%04x, %04x)\n", addr, addr + size);
	while (size) {
		int index = addr - curr_page;
		exist[index] = 1;
		buffer[index] = ram[addr];
		size--;
		addr++;
		num_elems++;
		if (addr == next_page) {
			debugger_print_helper(curr_page, buffer, exist);
			curr_page = next_page;
			next_page += 0x10;
			memset(exist, 0, sizeof(exist));
			num_elems = 0;
		}

	}
	if (num_elems) {
		debugger_print_helper(curr_page, buffer, exist);
	}
}

void debugger_diff_memory(uint16_t* ram1, uint16_t* ram2, uint16_t addr,
		uint16_t size) {
	uint16_t buffer1[16] = {0};
	uint16_t buffer2[16] = {0};
	uint16_t exist[16] = {0};
	uint16_t curr_page = addr & (~0xf);
	uint16_t next_page = curr_page + 0x10;
	size_t num_equals = 0;

	printf("MEMORY DIFF (%04x, %04x)\n", addr, addr + size);
	while (size) {
		int index = addr - curr_page;
		exist[index] = ram1[addr] != ram2[addr];
		if (exist[index]) {
			buffer1[index] = ram1[addr];
			buffer2[index] = ram2[addr];
			num_equals++;
		}
		size--;
		addr++;
		if (addr == next_page) {
			if (num_equals) {
				debugger_print_helper(curr_page, buffer1,
						exist);
				debugger_print_helper(curr_page, buffer2,
						exist);
			}
			curr_page = next_page;
			next_page += 0x10;
			memset(exist, 0, sizeof(exist));
			num_equals = 0;
		}
	}
	if (num_equals) {
		debugger_print_helper(curr_page, buffer1, exist);
		debugger_print_helper(curr_page, buffer2, exist);
	}
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

void debugger_save_stack(struct debugger* d, int save_pos) {
	if (save_pos < 0 || save_pos >= MAX_STACKS)
		return;
	if (d->stacks[save_pos]) {
		stack16_free(d->stacks[save_pos]);
	}
	d->stacks[save_pos] = stack16_duplicate(d->m->stack);
}

void debugger_compare_stacks(struct debugger* d, int pos0, int pos1) {
	stack16_show_compare(d->stacks[pos0], d->stacks[pos1]);
}

void debugger_save_memory(struct debugger* d, int save_pos) {
	(void)d;
	(void)save_pos;
	if (save_pos < 0 || save_pos > MAX_MEMORIES)
		return;
	if (d->rams[save_pos] == NULL)
		d->rams[save_pos] = malloc(sizeof(uint16_t) * (0x1<<15));
	memcpy(d->rams[save_pos], d->m->ram, sizeof(uint16_t) * (0x1<<15));
}

void debugger_load_memory(struct debugger* d, int save_pos) {
	(void)d;
	(void)save_pos;
	if (save_pos < 0 || save_pos > MAX_MEMORIES)
		return;
	if (d->rams[save_pos] == NULL) {
		printf("Invalid load position!\n");
		return;
	}
	memcpy(d->m->ram, d->rams[save_pos], sizeof(uint16_t) * (0x1<<15));
}

void debugger_compare_memory(struct debugger* d, int pos0, int pos1) {
	(void)d;
	(void)pos0;
	(void)pos1;
	if (pos0 < 0 || pos1 < 0 || pos0 >= MAX_MEMORIES ||
			pos1 >= MAX_MEMORIES) {
		return;
	}
	uint16_t *ram1 = d->rams[pos0];
	uint16_t *ram2 = d->rams[pos1];
	if (!ram1 || !ram2)
		return;
	debugger_diff_memory(ram1, ram2, 0, 0x8000);
}

