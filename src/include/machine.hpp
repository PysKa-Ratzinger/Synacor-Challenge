#pragma once

#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <memory>
#include <array>
#include <stack>

#define MAX_INPUT_SIZE 128

/*
 * struct machine: Represents the state of the virtual machine at any point
 * in time
 */
class Machine {
public:
	struct State {
		State();
		~State();

		std::array<uint16_t, 0x1 << 16> ram;
		std::array<uint16_t, 8> reg;
		std::stack<uint16_t> stack;
		uint16_t ip;
		size_t ticks;
	};

	class Debugger {
	public:
		virtual void beforeOp(Machine& m) = 0;
		virtual bool beforeHalted(Machine& m) = 0;

		const State& getState(const Machine& m);

		State& getState(Machine& m) {
			return const_cast<State&>(getState(
						const_cast<const Machine&>(m)));
		}

		uint16_t getIP(const Machine& m);
	};

	friend class Debugger;

	Machine();
	~Machine() {}

	bool tick();
	void run();

	size_t load_program(int fd);
	void attach_dbg(std::weak_ptr<Debugger> dbg);

private:
	uint16_t& get_reg(uint16_t a);
	uint16_t get_val(uint16_t a);

	bool Set (uint16_t a, uint16_t b);
	bool Push(uint16_t a);
	bool Pop (uint16_t a);
	bool Eq  (uint16_t a, uint16_t b, uint16_t c);
	bool Gt  (uint16_t a, uint16_t b, uint16_t c);
	bool Jmp (uint16_t a);
	bool Jnz (uint16_t a, uint16_t b);
	bool Jz  (uint16_t a, uint16_t b);
	bool Add (uint16_t a, uint16_t b, uint16_t c);
	bool Mult(uint16_t a, uint16_t b, uint16_t c);
	bool Mod (uint16_t a, uint16_t b, uint16_t c);
	bool And (uint16_t a, uint16_t b, uint16_t c);
	bool Or  (uint16_t a, uint16_t b, uint16_t c);
	bool Not (uint16_t a, uint16_t b);
	bool Rmem(uint16_t a, uint16_t b);
	bool Wmem(uint16_t a, uint16_t b);
	bool Call(uint16_t a);
	bool Ret ();
	bool Out (uint16_t a);
	bool In  (uint16_t a);
	bool Nop ();

	State m_state;

	std::weak_ptr<Debugger> m_dbg;
};

