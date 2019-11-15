#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "machine.hpp"
#include "machine_debug.hpp"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("USAGE: %s PROGRAM\n", argv[0]);
		return 1;
	}

	Machine m;
	std::shared_ptr<Debugger> dbg(new Debugger());

	int fd = open(argv[1], O_RDONLY);
	m.load_program(fd);
	close(fd);

	// m.attach_dbg(dbg);
	m.run();

	return 0;
}

