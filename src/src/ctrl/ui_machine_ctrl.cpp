#include "ctrl/ui_machine_ctrl.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>

static void readInputFromFD(int fd,
			std::function<void(const char* output)> out)
{
	char buffer[80];

	ssize_t nbytes;
	for (;;) {
		nbytes = read(fd, buffer, 80-1);
		if (nbytes < 0)
			break;

		buffer[nbytes] = '\0';
		out(buffer);
	}
}


MachineController::Comms::Comms()
{
	pipe(m_in_pipe);
	pipe(m_out_pipe);
	pipe(m_err_pipe);

	fcntl(m_out_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(m_err_pipe[0], F_SETFL, O_NONBLOCK);
}

MachineController::MachineController(
			std::function<void(const char* output)> out,
			std::function<void(const char* output)> err) :
	m_state(MachineController::state::NOT_RUNNING),
	m_machine(m_comms.m_in_pipe[0], m_comms.m_out_pipe[1],
			m_comms.m_err_pipe[1]),
	m_program_loaded(false),
	m_out(out),
	m_err(err)
{

}

MachineController::~MachineController()
{
	if (m_state != state::NOT_RUNNING) {
		this->stop_running();
	}
}

bool
MachineController::load_program(const char* filename)
{
	std::lock_guard<std::mutex> lock(m_mux);

	if (m_state != state::NOT_RUNNING)
		return false;

	int fd = open(filename, O_RDONLY);
	if (fd == -1)
		return false;

	if (m_machine.load_program(fd) > 0) {
		m_program_loaded = true;
	}

	close(fd);
	return true;
}

bool
MachineController::run_program()
{
	std::lock_guard<std::mutex> lock(m_mux);

	if (m_state != state::NOT_RUNNING)
		return false;

	if (!m_program_loaded)
		return false;

	m_program_loaded = false;
	m_thread = std::thread(&MachineController::behaviour, this);
	m_comms_bridge = std::thread(&MachineController::redirect_comms, this);
	m_state = state::RUNNING;
	return true;
}

bool
MachineController::stop_running()
{
	std::unique_lock<std::mutex> lock(m_mux);

	if (m_state != state::RUNNING)
		return true;

	m_state = state::CLOSING;
	m_machine.stop();
	kill(getpid(), EINTR);

	lock.unlock();

	m_thread.join();
	m_comms_bridge.join();

	return true;
}

bool
MachineController::send_input(const char* input, size_t nbytes)
{
	ssize_t n = write(this->m_comms.m_in_pipe[1], input, nbytes);
	return n > 0 && size_t(n) == nbytes;
}

void
MachineController::behaviour()
{
	m_machine.run(nullptr);

	std::unique_lock<std::mutex> lock(m_mux);
	m_state = state::NOT_RUNNING;
	m_cond.notify_all();
}

void
MachineController::redirect_comms()
{
	std::unique_lock<std::mutex> lock(m_mux);

	int epollfd = epoll_create1(0);

	{
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));

		ev.events = EPOLLIN;
		ev.data.u32 = 0;
		epoll_ctl(epollfd, EPOLL_CTL_ADD, m_comms.m_out_pipe[0], &ev);

		ev.data.u32 = 1;
		epoll_ctl(epollfd, EPOLL_CTL_ADD, m_comms.m_err_pipe[0], &ev);
	}

	struct epoll_event events[2];

	while (m_state == state::RUNNING) {
		lock.unlock();
		int n = epoll_wait(epollfd, events, 2, -1);

		if (n == 0 && errno == EINTR) {
			lock.lock();
			continue;
		}

		for (int i=0; i<n; i++) {
			if (events[i].data.u32 == 0) {
				readInputFromFD(m_comms.m_out_pipe[0], m_out);
			} else {
				readInputFromFD(m_comms.m_err_pipe[0], m_err);
			}
		}

		lock.lock();
	}
}

