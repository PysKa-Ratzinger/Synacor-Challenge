#include "machine_debug.hpp"
#include "common.hpp"
#include "machine.hpp"
#include "data_structures/stack.h"

#include <stdio.h>

#define CIRCULAR_SIZE 105
#define MAX_BREAKPOINTS 500
#define MAX_STATES 10
#define MAX_STACKS 10
#define MAX_MEMORIES 10

#define MAX_ADDR 0x7fff

static std::string op_to_mem_repr(uint16_t addr);

static std::string op_to_mem_repr(uint16_t addr) {
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

Debugger::Debugger() :
	m_states(MAX_STATES),
	m_stacks(MAX_STACKS),
	m_rams(MAX_MEMORIES),

	m_debug_opcodes(20),
	m_skips(0),
	m_dbg_enabled(true),
	m_dbg_stack(false),
	m_dbg_regs(false),
	m_dbg_disass(false),
	m_dbg_memory(false)
{

}

Debugger::~Debugger()
{

}

void
Debugger::setDebug(bool value)
{
	m_dbg_enabled = value;
}

void
Debugger::printStack(const Machine::State& s)
{
	std::stack<uint16_t> tmp = s.stack;

	printf("STACK TOP\n");
	while (tmp.size()) {
		uint16_t val = tmp.top();
		printf(": 0x%04x\n", val);
		tmp.pop();
	}
	printf("STACK BASE\n");
}

char get_printable(uint16_t val) {
	if (val >= 0x21 && val <= 0x7e) {
		return (char) val;
	}
	return '.';
}

void debugger_print_helper(char* buffer, size_t nbytes, uint16_t page,
		uint16_t* values, uint16_t* exist) {
	nbytes -= snprintf(buffer, nbytes, "%04x: ", page);
	for (int i=0; i<16; i++) {
		if (i == 8) {
			nbytes -= snprintf(buffer, nbytes, " ");
		}
		if (exist[i]) {
			nbytes -= snprintf(buffer, nbytes, "%04x ", values[i]);
		} else {
			nbytes -= snprintf(buffer, nbytes, "     ");
		}
	}
	nbytes -= snprintf(buffer, nbytes, "| ");
	for (int i=0; i<16; i++) {
		if (i == 8) {
			nbytes -= snprintf(buffer, nbytes, " ");
		}
		nbytes -= snprintf(buffer, nbytes, "%c", exist[i] ?
				get_printable(values[i]) : ' ');
	}
	nbytes -= snprintf(buffer, nbytes, " |\n");
	fflush(stdout);
}

void
Debugger::printMemory(const Machine::State& s, uint16_t addr,
		uint16_t size)
{
	uint16_t buffer[16] = {0};
	uint16_t exist[16] = {0};
	uint16_t curr_page = addr & (~0xf);
	uint16_t next_page = curr_page + 0x10;
	size_t num_elems = 0;

	char strbuffer[80];

	printf("MEMORY DUMP (%04x, %04x)\n", addr, addr + size);
	while (size) {
		int index = addr - curr_page;
		exist[index] = 1;
		buffer[index] = s.ram[addr];
		size--;
		addr++;
		num_elems++;
		if (addr == next_page) {
			debugger_print_helper(strbuffer, 80, curr_page,
					buffer, exist);
			printf("%s", strbuffer);
			curr_page = next_page;
			next_page += 0x10;
			memset(exist, 0, sizeof(exist));
			num_elems = 0;
		}

	}

	if (num_elems) {
		debugger_print_helper(strbuffer, 80, curr_page, buffer, exist);
		printf("%s", strbuffer);
	}
}

void
Debugger::printRegs(const Machine::State& s)
{
	auto& reg = s.reg;
	printf("R0: %04x, R1: %04x, R2: %04x, R3: %04x\n",
			reg[0], reg[1], reg[2], reg[3]);
	printf("R4: %04x, R5: %04x, R6: %04x, R7: %04x\n",
			reg[4], reg[5], reg[6], reg[7]);
	printf("IP: %04x    TICKS: %lu\n", s.ip, s.ticks);
}

void
Debugger::disassemble(const Machine::State& s, size_t opcodes)
{
	return disassemble(s, opcodes, s.ip);
}

void
Debugger::disassemble(const Machine::State& s, size_t opcodes, size_t ip)
{
	uint16_t op;

	std::string op_names[] = {
		"HALT", "SET", "PUSH", "POP", "EQ",  "GT", "JMP", "JNZ",
		"JZ",   "ADD", "MULT", "MOD", "AND", "OR", "NOT", "RMEM",
		"WMEM", "CALL", "RET", "OUT", "IN", "NOP"
	};

	uint16_t op_size[] = {
		0, 2, 1, 1, 3, 3, 1, 2,
		2, 3, 3, 3, 3, 3, 2, 2,
		2, 1, 0, 1, 1, 0
	};

	int currLine = 0;

	bool first = true;
	for (; opcodes--; ) {
		if (ip > MAX_ADDR)
			break;

		op = s.ram[ip];
		if (first) {
			m_disass_next_op_size = 1;
		}
		printf("0x%04lx: ", ip);
		if (op >= ARRAY_SIZE(op_names)) {
			printf("%04x%10s???\n", op, "");
		} else {
			size_t num_ops = op_size[op];
			if (first) {
				m_disass_next_op_size = num_ops + 1;
			}
			printf("%-4s", op_names[op].c_str());
			for (size_t i=0; i<num_ops; i++) {
				printf(" %-5s", op_to_mem_repr(s.ram[ip+1+i])
						.c_str());
			}
			printf("\n");
			ip += num_ops;
		}
		ip++;
		currLine++;
		first = false;
	}
}

void
Debugger::dumpState(const Machine::State& s)
{
	if (m_dbg_memory) {
		printMemory(s, m_memory_pos, 16 * 32);
	}

	if (m_dbg_stack) {
		printStack(s);
	}

	if (m_dbg_regs) {
		printRegs(s);
	}

	if (m_dbg_disass) {
		disassemble(s, m_debug_opcodes, m_disass_pos);
	}
}

void
Debugger::listBreakpoints()
{
	printf("BREAKPOINTS: \n");
	if (m_breakpoints.size() == 0) {
		printf("   EMPTY\n");
	} else {
		for (uint16_t p : m_breakpoints) {
			printf(" + %04x\n", p);
		}
	}
}

void
Debugger::saveStack(const Machine::State& s, size_t save_pos)
{
	if (save_pos >= m_stacks.size())
		return;
	this->m_stacks[save_pos] = {true, s.stack};
}

void
Debugger::compareStacks(size_t pos0, size_t pos1)
{
	if (!m_stacks.at(pos0).first) {
		std::cerr << "No stack found at position=" << pos0 << std::endl;
	}

	if (!m_stacks.at(pos1).first) {
		std::cerr << "No stack found at position=" << pos1 << std::endl;
	}

	stack_show_compare(m_stacks.at(pos0).second, m_stacks.at(pos1).second);
}

void
Debugger::compareMemory(size_t pos0, size_t pos1, uint16_t addr, uint16_t size)
{
	if (pos0 > m_rams.size() || pos1 > m_rams.size())
		return;

	if (!m_rams.at(pos0).first || !m_rams.at(pos1).first)
		return;

	auto& ram1 = m_rams.at(pos0).second;
	auto& ram2 = m_rams.at(pos1).second;

	uint16_t buffer1[16] = {0};
	uint16_t buffer2[16] = {0};
	uint16_t exist[16] = {0};
	uint16_t curr_page = addr & (~0xf);
	uint16_t next_page = curr_page + 0x10;
	size_t num_equals = 0;

	char buffer[80];

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
				debugger_print_helper(buffer, 80, curr_page,
						buffer1, exist);
				debugger_print_helper(buffer, 80, curr_page,
						buffer2, exist);
			}
			curr_page = next_page;
			next_page += 0x10;
			memset(exist, 0, sizeof(exist));
			num_equals = 0;
		}
	}
	if (num_equals) {
		debugger_print_helper(buffer, 80, curr_page, buffer1, exist);
		debugger_print_helper(buffer, 80, curr_page, buffer2, exist);
	}
}

void
Debugger::saveState(const Machine::State& s, size_t save_pos)
{
	if (save_pos > m_states.size())
		return;
	this->m_states.at(save_pos).first = true;
	this->m_states.at(save_pos).second = s;
}

void
Debugger::loadState(Machine::State& s, size_t save_pos)
{
	if (save_pos > m_states.size())
		return;
	if (!m_states.at(save_pos).first)
		return;
	s = m_states.at(save_pos).second;
}

void
Debugger::saveMemory(const Machine::State& s, size_t save_pos)
{
	if (save_pos > m_rams.size())
		return;
	this->m_rams.at(save_pos) = { true, s.ram };
}

void
Debugger::loadMemory(Machine::State& s, size_t save_pos)
{
	if (save_pos > m_rams.size())
		return;

	if (m_rams.at(save_pos).first == false) {
		printf("Invalid load position.\n");
		return;
	}

	s.ram = m_rams.at(save_pos).second;
}

void
Debugger::setBreakpoint(uint16_t ip, bool active)
{
	if (active) {
		this->m_breakpoints.insert(ip);
	} else {
		auto it = this->m_breakpoints.find(ip);
		if (it != this->m_breakpoints.end()) {
			this->m_breakpoints.erase(it);
		}
	}
}

bool
Debugger::shell(Machine::State& s)
{
	static char prev_buffer[256];

	bool res = true;
	while (true) {
		this->dumpState(s);

		printf("(debug) ");

		char buffer[256];
		fgets(buffer, 256, stdin);

		char *cmd = buffer;

		if (buffer[0] == '\n') {
			cmd = prev_buffer;
		} else {
			memcpy(prev_buffer, buffer, sizeof(256));
		}

		if (strncmp(cmd, "stack_on", 8) == 0) {
			m_dbg_stack = true;

		} else if (strncmp(cmd, "stack_off", 9) == 0) {
			m_dbg_stack = false;

		} else if (strncmp(cmd, "stack_save", 10) == 0) {
			int save_pos = strtol(cmd + 11, NULL, 10);
			// TODO: Continue here
			this->saveStack(s, save_pos);

		} else if (strncmp(cmd, "stack_compare", 13) == 0) {
			char *endstr = NULL;
			int pos0 = strtol(cmd + 14, &endstr, 10);
			int pos1 = strtol(endstr, NULL, 10);
			this->compareStacks(pos0, pos1);

		} else if (strncmp(cmd, "regs_on", 7) == 0) {
			this->m_dbg_regs = 1;

		} else if (strncmp(cmd, "regs_off", 8) == 0) {
			this->m_dbg_regs = 0;

		} else if (strncmp(cmd, "disass_on", 9) == 0) {
			this->m_dbg_disass = 1;

		} else if (strncmp(cmd, "disass_off", 10) == 0) {
			this->m_dbg_disass = 0;

		} else if (strncmp(cmd, "memory_on", 9) == 0) {
			this->m_dbg_memory = 1;

		} else if (strncmp(cmd, "memory_off", 10) == 0) {
			this->m_dbg_memory = 0;

		} else if (strncmp(cmd, "memory_save", 11) == 0) {
			int save_pos = strtol(cmd + 12, NULL, 10);
			this->saveMemory(s, save_pos);

		} else if (strncmp(cmd, "memory_load", 11) == 0) {
			int save_pos = strtol(cmd + 12, NULL, 10);
			this->loadMemory(s, save_pos);

		} else if (strncmp(cmd, "memory_cmp", 10) == 0) {
			char *endstr = NULL;
			int pos0 = strtol(cmd + 11, &endstr, 10);
			int pos1 = strtol(endstr, NULL, 10);
			this->compareMemory(pos0, pos1, 0, 0x800);

		} else if (strncmp(cmd, "dump", 4) == 0) {
			size_t addr = strtol(cmd+5, NULL, 16);
			this->m_disass_pos = addr;

		} else if (strncmp(cmd, "dops", 4) == 0) {
			this->m_debug_opcodes = strtol(cmd + 5, NULL, 16);

		} else if (strncmp(cmd, "save", 4) == 0) {
			size_t pos = strtol(cmd+5, NULL, 10);
			this->saveState(s, pos);

		} else if (strncmp(cmd, "load", 4) == 0) {
			size_t pos = strtol(cmd+5, NULL, 10);
			this->loadState(s, pos);

		} else if (strncmp(cmd, "s", 1) == 0) {
			this->m_sskips = strtol(cmd+2, NULL, 10);
			break;

		} else if (strncmp(cmd, "b", 1) == 0) {
			this->setBreakpoint(strtol(cmd+2, NULL, 16), true);

		} else if (strncmp(cmd, "ub", 2) == 0) {
			this->setBreakpoint(strtol(cmd+3, NULL, 16), false);

		} else if (strncmp(cmd, "lb", 2) == 0) {
			this->listBreakpoints();

		} else if (strncmp(cmd, "c", 1) == 0) {
			this->m_dbg_enabled = false;
			this->m_skips = strtol(cmd+2, NULL, 10);
			break;

		} else if (strncmp(cmd, "p", 1) == 0) {
			uint16_t start_pos = strtol(cmd+2, NULL, 16);
			this->m_memory_pos = start_pos;

		} else if (strncmp(cmd, "q", 1) == 0) {
			res = false;
			break;

		} else if (strncmp(cmd, "halt", 4) == 0) {
			this->halt();

		} else {
			this->dumpState(s);
		}
	}

	return res;
}

void
Debugger::beforeOp(Machine& m)
{
	Machine::State& s = getState(m);
	this->m_disass_pos = s.ip;

	if (m_dbg_enabled) {
		if (this->m_sskips > 0) {
			this->m_sskips--;
		} else {
			this->shell(s);
		}
	} else {
		auto it = this->m_breakpoints.find(s.ip);
		if (it != this->m_breakpoints.end()) {
			if (m_skips > 0) {
				m_skips--;
			} else {
				this->setDebug(true);
				this->shell(s);
			}
		}
	}
}

bool
Debugger::beforeHalted(Machine& m)
{
	Machine::State& s = getState(m);
	this->m_disass_pos = s.ip;
	this->m_skips = 0;
	this->m_sskips = 0;
	this->setDebug(true);
	this->shell(s);
	return true;
}

void
Debugger::halt()
{

}

void
Debugger::run(Machine& m)
{
	m.run(this);
}

