#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtkmm.h>

#include "ui_machine.hpp"

int main(int argc, char* argv[]) {

	/*
	if (argc != 2) {
		printf("USAGE: %s PROGRAM\n", argv[0]);
		return 1;
	}
	*/

	auto app = Gtk::Application::create(argc, argv, "org.raztinger.pyska");

	UiMachine ui;

	app->run(ui, argc, argv);

	/*
	Machine m(stdin, stdout, stderr);
	std::shared_ptr<Debugger> dbg(new Debugger());

	int fd = open(argv[1], O_RDONLY);
	m.load_program(fd);
	close(fd);

	dbg->run(m);
	*/

	return 0;
}

