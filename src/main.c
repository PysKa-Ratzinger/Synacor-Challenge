#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "stack.h"
#include "machine.h"
#include "machine_debug.h"

int main() {
	struct machine* m = machine_new();
	int fd = open("../challenge.bin", O_RDONLY);
	machine_load_program(m, fd);
	close(fd);

	struct debugger* dbg = machine_debugger_create();
	machine_attach_debugger(m, dbg);
	machine_run(m);

	machine_free(m);
	return 0;
}

