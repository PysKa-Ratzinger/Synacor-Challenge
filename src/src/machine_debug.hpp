#pragma once

#include "machine.hpp"

#include <vector>
#include <set>

class Debugger : public Machine::Debugger {
public:
	Debugger();
	~Debugger();

	bool beforeHalted(Machine& m) override;
	void beforeOp(Machine& m) override;

	bool shell(Machine::State& s);

	void printStack(const Machine::State& m);
	void printMemory(const Machine::State& m, uint16_t addr,
			uint16_t size);
	void printRegs(const Machine::State& m);

	void dumpState(const Machine::State& m);

	void disassemble(const Machine::State& m, size_t opcodes);
	void disassemble(const Machine::State& m, size_t opcodes,
			size_t ip);

	void setBreakpoint(uint16_t ip, bool active);
	void listBreakpoints();

	void setDebug(bool value);

	void compareStacks(size_t pos0, size_t pos1);
	void compareMemory(size_t pos0, size_t pos1, uint16_t addr,
			uint16_t size);

	void saveState(const Machine::State& m, size_t pos);
	void saveStack(const Machine::State& m, size_t pos);
	void saveMemory(const Machine::State& m, size_t pos);

	void loadState(Machine::State&, size_t pos);
	void loadStack(Machine::State&, size_t pos);
	void loadMemory(Machine::State&, size_t pos);

	void halt();
	void run(Machine& m);

private:
	std::vector<std::pair<bool, Machine::State>> m_states;
	std::vector<std::pair<bool, std::stack<uint16_t>>> m_stacks;
	std::vector<std::pair<bool, std::array<uint16_t, 1 << 16>>> m_rams;
	std::set<uint16_t> m_breakpoints;

	size_t m_debug_opcodes;
	size_t m_skips;
	size_t m_sskips;
	bool m_dbg_enabled;
	bool m_dbg_stack;
	bool m_dbg_regs;
	bool m_dbg_disass;
	bool m_dbg_memory;

	size_t m_disass_pos = 0;
	size_t m_disass_next_op_size = 0;
	size_t m_memory_pos = 0;
};

void machine_dump(struct machine* machine);
void debugger_diff_memory(uint16_t* ram1, uint16_t* ram2, uint16_t addr,
		uint16_t size);
void debugger_compare_stacks(struct debugger* d, int pos0, int pos1);
void debugger_compare_memory(struct debugger* d, int pos0, int pos1);

