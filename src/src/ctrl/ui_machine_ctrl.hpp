#ifndef UI_MACHINE_CTRL_HPP
#define UI_MACHINE_CTRL_HPP

#include "machine.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class MachineController {
public:
	MachineController(
			std::function<void(const char* output)> out,
			std::function<void(const char* output)> err);
	~MachineController();

	struct Comms {
		Comms();

		int m_in_pipe[2];
		int m_out_pipe[2];
		int m_err_pipe[2];
	};

	enum class state {
		NOT_RUNNING, RUNNING, CLOSING
	};

	bool load_program(const char* filename);
	bool run_program();
	bool stop_running();
	bool send_input(const char* input, size_t nbytes);

private:
	void behaviour();
	void redirect_comms();

	MachineController::state m_state;

	Comms m_comms;
	Machine m_machine;
	bool m_program_loaded;

	std::function<void(const char* output)> m_out;
	std::function<void(const char* output)> m_err;

	std::thread m_thread;
	std::thread m_comms_bridge;
	std::mutex m_mux;
	std::condition_variable m_cond;
};

#endif  // UI_MACHINE_CTRL_HPP

