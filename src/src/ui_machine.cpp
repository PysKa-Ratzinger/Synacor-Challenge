#include "ui_machine.hpp"

#include <stdlib.h>
#include <gtkmm.h>
#include <stdexcept>
#include <functional>

UiMachine::UiMachine() :
	m_builder(Gtk::Builder::create_from_file("./gui/main.ui"))
{
	Gtk::Box* cont;
	m_builder->get_widget("container", cont);
	if (cont) {
		add(*cont);
	}

	auto obj = m_builder->get_object("bar_run");
	auto menu_item = Glib::RefPtr<Gtk::MenuItem>::cast_dynamic(obj);
	menu_item->signal_activate().connect_notify(
				std::bind(&UiMachine::run_program, this));

	set_title("Simple Demo");
	set_default_size(800, 400);
	show_all();
}

UiMachine::~UiMachine()
{
}

void
UiMachine::run_program()
{
	printf("HI\n");
}

