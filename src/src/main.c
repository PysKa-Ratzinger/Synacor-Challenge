#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "machine.h"
#include "machine_debug.h"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("USAGE: %s PROGRAM\n", argv[0]);
		return 1;
	}

	char *program = argv[1];

	struct machine* m = machine_new();
	int fd = open(program, O_RDONLY);
	machine_load_program(m, fd);
	close(fd);

	struct debugger* dbg = machine_debugger_create();
	machine_attach_debugger(m, dbg);
	machine_run(m);

	machine_free(m);
	return 0;
}

