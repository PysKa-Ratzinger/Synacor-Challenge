#include "machine_debug.h"
#include "common.h"
#include "stack.h"

#include <stdio.h>

#define CIRCULAR_SIZE 105

struct debugger {
	struct machine* m;
	struct circular_array_16* ip_history;

	size_t debug_opcodes;
	char debug_ip_history;
	char debug_stack;
	char debug_regs;
	char debug_disass;
	char debug_memory;
	char debug_enabled;
};

struct debugger* machine_debugger_create() {
	struct debugger* res = malloc(sizeof(*res));
	memset(res, 0, sizeof(*res));
	res->ip_history = circular_array_create(CIRCULAR_SIZE);
	res->debug_enabled = 1;
	return res;
}

void machine_debugger_free(struct debugger* d) {
	free(d);
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
		snprintf(res, 50, "M(%04x)", addr);
		return res;
	}

	addr &= 0x7fff;
	if (addr <= 7) {
		snprintf(res, 50, "R%u", addr);
	} else {
		snprintf(res, 50, "?%04x?", addr);
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
			size_t bytes = 0;
			bytes += printf("%04x", op);
			for (size_t i=0; i<num_ops; i++) {
				bytes += printf(" %04x", fake_ip[i+1]);
			}
			while (bytes < 22) {
				bytes += printf(" ");
			}
			printf(" ");
			printf("%-4s", op_names[op]);
			for (size_t i=0; i<num_ops; i++) {
				printf(" %-7s", machine_get_mem_repr(fake_ip[1+i]));
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

void debugger_tick(struct debugger* d) {
	circular_array_insert(d->ip_history, d->m->ip - d->m->ram);

	if (d->debug_enabled) {
		char buffer[256];
		fgets(buffer, sizeof(buffer), stdin);
	}
	return;
}

