#include "machine.h"
#include "machine_debug.h"
#include "common.h"
#include "stack.h"

#include <unistd.h>

#define HALT 0
#define SET  1
#define PUSH 2
#define POP  3
#define EQ   4
#define GT   5
#define JMP  6
#define JNZ  7
#define JZ   8
#define ADD  9
#define MULT 10
#define MOD  11
#define AND  12
#define OR   13
#define NOT  14
#define RMEM 15
#define WMEM 16
#define CALL 17
#define RET  18
#define OUT  19
#define IN   20
#define NOP  21

#define ASSERT_REG(x) {if ((x)<= 0x7fff || ((x)&0x7fff)>7) { \
	printf("Invalid REG! (%04x)\n", (x)); return 1;}}
#define ASSERT_VALID(x) {if ((x)>0x7fff+8) { \
	printf("Invalid VAL! (%04x)\n", (x)); return 1;}}
#define MACHINE_REG(m, a) ((m)->reg[(a)&0x7fff])
#define MACHINE_VAL(m, b) (((b) <= 0x7fff) ? (b) : MACHINE_REG((m),(b)))
#define CAP(x) ((x)&0x7fff)

struct machine* machine_new() {
	struct machine* res = NULL;
	res = (struct machine*) malloc(sizeof(struct machine));
	if (res != NULL) {
		memset(res, 0, sizeof(struct machine));
		res->stack = stack16_create();
	}
	return res;
}

void machine_free(struct machine* machine) {
	stack16_free(machine->stack);
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
	memset(machine->reg, 0, sizeof(machine->reg));
	machine->ip = machine->ram;
	return total_bytes;
}

char machine_set(struct machine* m, uint16_t a, uint16_t b) {
	ASSERT_REG(a);
	MACHINE_REG(m,a) = MACHINE_VAL(m,b);
	m->ip += 3;
	return 0;
}

char machine_add(struct machine* m, uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	MACHINE_REG(m,a) = CAP(MACHINE_VAL(m,b) + MACHINE_VAL(m,c));
	m->ip += 4;
	return 0;
}

char machine_eq(struct machine* m, uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	MACHINE_REG(m,a) = MACHINE_VAL(m,b) == MACHINE_VAL(m,c);
	m->ip += 4;
	return 0;
}

char machine_push(struct machine* m, uint16_t a) {
	ASSERT_VALID(a);
	stack16_push(m->stack, MACHINE_VAL(m,a));
	m->ip += 2;
	return 0;
}

char machine_pop(struct machine* m, uint16_t a) {
	ASSERT_REG(a);
	stack16_pop(m->stack, &MACHINE_REG(m,a));
	m->ip += 2;
	return 0;
}

char machine_gt(struct machine* m, uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	MACHINE_REG(m,a) = MACHINE_VAL(m,b) > MACHINE_VAL(m,c);
	m->ip += 4;
	return 0;
}

char machine_jmp(struct machine* m, uint16_t a) {
	ASSERT_VALID(a);
	m->ip = &m->ram[MACHINE_VAL(m,a)];
	return 0;
}

char machine_jz(struct machine* m, uint16_t a, uint16_t b) {
	ASSERT_VALID(a);
	if (MACHINE_VAL(m, a) == 0) {
		return machine_jmp(m, b);
	} else {
		m->ip += 3;
	}
	return 0;
}

char machine_jnz(struct machine* m, uint16_t a, uint16_t b) {
	ASSERT_VALID(a);
	if (MACHINE_VAL(m, a) != 0) {
		return machine_jmp(m, b);
	} else {
		m->ip += 3;
	}
	return 0;
}

char machine_and(struct machine* m, uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	MACHINE_REG(m,a) = CAP(MACHINE_VAL(m,b) & MACHINE_VAL(m,c));
	m->ip += 4;
	return 0;
}

char machine_or(struct machine* m, uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	MACHINE_REG(m,a) = CAP(MACHINE_VAL(m,b) | MACHINE_VAL(m,c));
	m->ip += 4;
	return 0;
}

char machine_not(struct machine* m, uint16_t a, uint16_t b) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	MACHINE_REG(m,a) = CAP(~MACHINE_VAL(m,b));
	m->ip += 3;
	return 0;
}

char machine_call(struct machine* m, uint16_t a) {
	ASSERT_VALID(a);
	machine_push(m, m->ip - m->ram + 2);
	machine_jmp(m, a);
	return 0;
}

char machine_mult(struct machine* m, uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	MACHINE_REG(m,a) = CAP(MACHINE_VAL(m,b) * MACHINE_VAL(m,c));
	m->ip += 4;
	return 0;
}

char machine_mod(struct machine* m, uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	MACHINE_REG(m,a) = CAP(MACHINE_VAL(m,b) % MACHINE_VAL(m,c));
	m->ip += 4;
	return 0;
}

char machine_rmem(struct machine* m, uint16_t a, uint16_t b) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	MACHINE_REG(m,a) = CAP(m->ram[MACHINE_VAL(m,b)]);
	m->ip += 3;
	return 0;
}

char machine_wmem(struct machine* m, uint16_t a, uint16_t b) {
	ASSERT_VALID(a);
	ASSERT_VALID(b);
	m->ram[MACHINE_VAL(m,a)] = MACHINE_VAL(m,b);
	m->ip += 3;
	return 0;
}

char machine_ret(struct machine* m) {
	uint16_t val;
	stack16_pop(m->stack, &val);
	m->ip = &m->ram[val];
	return 0;
}

char machine_out(struct machine* m, uint16_t a) {
	ASSERT_VALID(a);
	printf("%c", MACHINE_VAL(m, a));
	fflush(stdout);
	m->ip += 2;
	return 0;
}

char machine_nop(struct machine* m) {
	m->ip++;
	return 0;
}

int machine_tick(struct machine* machine) {
	uint16_t *op = machine->ip;

	if (machine->debugger) {
		debugger_tick(machine->debugger);
	}

	if (*op > 21) {
		fprintf(stderr, "Invalid op: %04x\n", *op);
		machine_dump(machine);
		return -1;
	}

	switch (*op) {
		/* Case 0 */
		case HALT: return 1;
		case SET:  return machine_set( machine, op[1], op[2]);
		case PUSH: return machine_push(machine, op[1]);
		case POP:  return machine_pop( machine, op[1]);
		case EQ:   return machine_eq(  machine, op[1], op[2], op[3]);
		case GT:   return machine_gt(  machine, op[1], op[2], op[3]);
		case JMP:  return machine_jmp( machine, op[1]);
		case JNZ:  return machine_jnz( machine, op[1], op[2]);
		case JZ:   return machine_jz(  machine, op[1], op[2]);
		case ADD:  return machine_add( machine, op[1], op[2], op[3]);
		case MULT: return machine_mult(machine, op[1], op[2], op[3]);
		case MOD:  return machine_mod( machine, op[1], op[2], op[3]);
		case AND:  return machine_and( machine, op[1], op[2], op[3]);
		case OR:   return machine_or(  machine, op[1], op[2], op[3]);
		case NOT:  return machine_not( machine, op[1], op[2]);
		case RMEM: return machine_rmem(machine, op[1], op[2]);
		case WMEM: return machine_wmem(machine, op[1], op[2]);
		case CALL: return machine_call(machine, op[1]);
		case RET:  return machine_ret( machine);
		case OUT:  return machine_out( machine, op[1]);
		case NOP:  return machine_nop( machine);
		default:   return -1;
	}
}

void machine_run(struct machine* machine) {
	while (machine_tick(machine) == 0);
}

