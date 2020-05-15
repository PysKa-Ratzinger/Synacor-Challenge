#include "machine.hpp"
#include "machine_debug.hpp"
#include "common.hpp"

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <signal.h>

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
	dprintf(m_err, "Invalid REG! (%04x)\n", (x)); return 1;}}
#define ASSERT_VALID(x) {if ((x)>0x7fff+8) { \
	dprintf(m_err, "Invalid VAL! (%04x)\n", (x)); return 1;}}
#define CAP(x) ((x)&0x7fff)

Machine::State::State() :
	ram({0}),
	reg({0}),
	stack(),
	ip(0),
	ticks(0)
{

}

Machine::State::~State() {}

const Machine::State&
Machine::Debugger::getState(const Machine& m)
{
	return m.m_state;
}
uint16_t
Machine::Debugger::getIP(const Machine& m)
{
	return m.m_state.ip;
}

Machine::Machine(int in, int out, int err) :
	m_in(in), m_out(out), m_err(err)
{

}

size_t
Machine::load_program(int fd) {
	ssize_t bytes_read;
	size_t total_bytes = 0;
	size_t n_left = m_state.ram.size();
	while ((bytes_read = read(fd, &m_state.ram.at(total_bytes), n_left))) {
		total_bytes += bytes_read;
		n_left -= bytes_read;
		if (n_left == 0)
			break;
	}
	m_state.reg = {0};
	m_state.ip = 0;
	return total_bytes;
}

uint16_t&
Machine::get_reg(uint16_t a)
{
	return m_state.reg.at(a&0x7fff);
}

uint16_t
Machine::get_val(uint16_t a)
{
	return (a <= 0x7fff) ? a : get_reg(a);
}

bool
Machine::readline()
{
	std::unique_lock<std::mutex> lock(m_mux);

	size_t offset = 0;
	ssize_t nbytes = 0;

	while (offset < MAX_INPUT_SIZE - 1 && !m_stop_flag) {
		lock.unlock();
		nbytes = read(m_in, m_state.buffer + offset, 1);
		lock.lock();

		if (nbytes < 0) {
			if (errno == EINTR) {
				continue;
			}

			return false;
		}

		if (m_state.buffer[offset] == '\n') {
			m_state.buffer[offset+1] = '\0';
			m_state.buffer_sz = offset + 1;
			m_state.buffer_offset = 0;
			return true;
		}

		offset++;
	}

	return false;
}

bool
Machine::Set(uint16_t a, uint16_t b) {
	ASSERT_REG(a);
	get_reg(a) = get_val(b);
	m_state.ip += 3;
	return true;
}

bool
Machine::Add(uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	get_reg(a) = CAP(get_val(b) + get_val(c));
	m_state.ip += 4;
	return true;
}

bool
Machine::Eq(uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	get_reg(a) = get_val(b) == get_val(c);
	m_state.ip += 4;
	return true;
}

bool
Machine::Push(uint16_t a) {
	ASSERT_VALID(a);
	m_state.stack.push(get_val( a));
	m_state.ip += 2;
	return true;
}

bool
Machine::Pop(uint16_t a) {
	ASSERT_REG(a);
	get_reg(a) = m_state.stack.top();
	m_state.stack.pop();
	m_state.ip += 2;
	return true;
}

bool
Machine::Gt(uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	get_reg(a) = get_val(b) > get_val(c);
	m_state.ip += 4;
	return true;
}

bool
Machine::Jmp(uint16_t a) {
	ASSERT_VALID(a);
	m_state.ip = get_val(a);
	return true;
}

bool
Machine::Jz(uint16_t a, uint16_t b) {
	ASSERT_VALID(a);
	if (get_val(a) == 0) {
		return Jmp(b);
	} else {
		m_state.ip += 3;
	}
	return true;
}

bool
Machine::Jnz(uint16_t a, uint16_t b) {
	ASSERT_VALID(a);
	if (get_val(a) != 0) {
		return Jmp(b);
	} else {
		m_state.ip += 3;
	}
	return true;
}

bool
Machine::And(uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	get_reg(a) = CAP(get_val(b) & get_val(c));
	m_state.ip += 4;
	return true;
}

bool
Machine::Or(uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	get_reg(a) = CAP(get_val(b) | get_val(c));
	m_state.ip += 4;
	return true;
}

bool
Machine::Not(uint16_t a, uint16_t b) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	get_reg(a) = CAP(~get_val(b));
	m_state.ip += 3;
	return true;
}

bool
Machine::Call(uint16_t a) {
	ASSERT_VALID(a);
	Push(m_state.ip + 2);
	Jmp(a);
	return true;
}

bool
Machine::Mult(uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	get_reg(a) = CAP(get_val(b) * get_val(c));
	m_state.ip += 4;
	return true;
}

bool
Machine::Mod(uint16_t a, uint16_t b, uint16_t c) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	ASSERT_VALID(c);
	get_reg(a) = CAP(get_val(b) % get_val(c));
	m_state.ip += 4;
	return true;
}

bool
Machine::Rmem(uint16_t a, uint16_t b) {
	ASSERT_REG(a);
	ASSERT_VALID(b);
	get_reg(a) = CAP(m_state.ram.at(get_val(b)));
	m_state.ip += 3;
	return true;
}

bool
Machine::Wmem(uint16_t a, uint16_t b) {
	ASSERT_VALID(a);
	ASSERT_VALID(b);
	m_state.ram.at(get_val(a)) = get_val(b);
	m_state.ip += 3;
	return true;
}

bool
Machine::Ret() {
	uint16_t val = m_state.stack.top();
	m_state.stack.pop();
	m_state.ip = val;
	return true;
}

bool
Machine::Out(uint16_t a) {
	ASSERT_VALID(a);
	dprintf(m_out, "%c", get_val(a));
	m_state.ip += 2;
	return true;
}

bool
Machine::In(uint16_t a) {
	ASSERT_VALID(a);
	if (m_state.buffer_offset == m_state.buffer_sz &&
			this->readline() == false)
		return false;
	get_reg(a) = m_state.buffer[m_state.buffer_offset];
	m_state.buffer_offset++;
	m_state.ip += 2;
	return true;
}

bool
Machine::Nop() {
	m_state.ip++;
	return true;
}

bool Machine::tick(Debugger* dbg) {
	uint16_t &op = m_state.ram.at(m_state.ip);
	m_state.ticks++;

	if (dbg) {
		if (op == IN) {
			// If call to IN would block
			if (m_state.buffer_offset == m_state.buffer_sz) {
				dbg->beforeHalted(*this);
			}
		} else {
			dbg->beforeOp(*this);
		}
	}

	if (op > 21) {
		dprintf(m_err, "Invalid op: %04x\n", op);
		// machine_dump(machine);
		return false;
	}

	uint16_t* p = &op;
	switch (op) {
		case HALT:
			dprintf(m_out, "Program halted!\n");
			if (dbg) {
				return dbg->beforeHalted(*this);
			} else {
				return false;
			}

		case SET:  return Set (p[1], p[2]);
		case PUSH: return Push(p[1]);
		case POP:  return Pop (p[1]);
		case EQ:   return Eq  (p[1], p[2], p[3]);
		case GT:   return Gt  (p[1], p[2], p[3]);
		case JMP:  return Jmp (p[1]);
		case JNZ:  return Jnz (p[1], p[2]);
		case JZ:   return Jz  (p[1], p[2]);
		case ADD:  return Add (p[1], p[2], p[3]);
		case MULT: return Mult(p[1], p[2], p[3]);
		case MOD:  return Mod (p[1], p[2], p[3]);
		case AND:  return And (p[1], p[2], p[3]);
		case OR:   return Or  (p[1], p[2], p[3]);
		case NOT:  return Not (p[1], p[2]);
		case RMEM: return Rmem(p[1], p[2]);
		case WMEM: return Wmem(p[1], p[2]);
		case CALL: return Call(p[1]);
		case RET:  return Ret ();
		case OUT:  return Out (p[1]);
		case IN:   return In  (p[1]);
		case NOP:  return Nop ();
		default:   return -1;
	}
}

void
Machine::run(Debugger* dbg)
{
	while (tick(dbg));
}

void
Machine::stop()
{
	std::lock_guard<std::mutex> lock(m_mux);
	m_stop_flag = true;
	kill(getpid(), EINTR);
}

