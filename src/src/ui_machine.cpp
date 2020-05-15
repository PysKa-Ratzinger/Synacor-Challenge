#include "ui_machine.hpp"

#include <stdlib.h>
#include <gtkmm.h>
#include <stdexcept>
#include <functional>
#include <signal.h>
#include <unistd.h>

#include <mutex>

#include <gdk/gdkkeysyms.h>

UiMachine::UiMachine() :
	m_builder(Gtk::Builder::create_from_file("./gui/main.ui")),
	m_ctrl(
			std::bind(&UiMachine::handle_output, this, 0,
				std::placeholders::_1),
			std::bind(&UiMachine::handle_output, this, 1,
				std::placeholders::_1))
{
	Gtk::Box* cont;
	m_builder->get_widget("container", cont);
	if (cont) {
		add(*cont);
	}

	this->signal_key_press_event().connect_notify(
			std::bind(&UiMachine::key_pressed, this,
				std::placeholders::_1));

	auto obj = m_builder->get_object("bar_run");
	auto menu_item = Glib::RefPtr<Gtk::MenuItem>::cast_dynamic(obj);
	menu_item->signal_activate().connect_notify(
				std::bind(&UiMachine::run_program, this));

	obj = m_builder->get_object("bar_open");
	menu_item = Glib::RefPtr<Gtk::MenuItem>::cast_dynamic(obj);
	menu_item->signal_activate().connect_notify(
				std::bind(&UiMachine::load_program, this));

	obj = m_builder->get_object("app_output");
	m_text_window = Glib::RefPtr<Gtk::TextView>::cast_dynamic(obj);

	obj = m_builder->get_object("user_input");
	m_user_input = Glib::RefPtr<Gtk::Entry>::cast_dynamic(obj);

	set_title("Simple Demo");
	set_default_size(800, 400);
	show_all();
}

UiMachine::~UiMachine()
{
	this->stop_running();
}

void
UiMachine::run_program()
{
	if (!this->m_ctrl.run_program()) {
		this->handle_output(1, "OH NO!!!\n");
	}
}

void
UiMachine::load_program()
{
	Gtk::FileChooserDialog dialog(*this,
			"Open Binary",
			Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Open", Gtk::RESPONSE_ACCEPT);

	int res = dialog.run();

	if (res == Gtk::RESPONSE_ACCEPT) {
		std::string filename = dialog.get_filename();
		if (!m_ctrl.load_program(filename.c_str())) {
			Gtk::MessageDialog dialog(*this,
					"Could not open file: " + filename);
			dialog.run();
		} else {
			this->handle_output(2, "File opened successfully\n");
		}
	}
}

void
UiMachine::stop_running()
{
	m_ctrl.stop_running();
}

void
UiMachine::key_pressed(GdkEventKey* event)
{
	if (event->type != GDK_KEY_PRESS)
		return;

	switch (event->keyval) {
	case GDK_KEY_Return:
		Glib::signal_idle().connect([this]() {
			auto text = this->m_user_input->get_text();
			std::string input = text + "\n";
			this->m_ctrl.send_input(input.c_str(), input.size());
			this->m_user_input->set_text("");
			return false;
		});
		break;

	default:
		break;
	}
}

void
UiMachine::handle_output(int type, const char* output)
{
	const char* color = nullptr;
	switch (type) {
	case 0:
		break;
	case 1:
		color = "darkred";
		break;
	case 2:
		color = "green";
		break;
	default:
		color = "red";
		break;
	}

	std::string msg(output);
	Glib::signal_idle().connect([this, color, msg]() {
		auto buffer = m_text_window->get_buffer();

		if (color) {
			std::stringstream ss;
			ss << "<span color=\"" << color << "\">" << msg;
			ss << "</span>";
			buffer->insert_markup(buffer->end(), ss.str());
		} else {
			buffer->insert(buffer->end(), msg);
		}

		Glib::signal_idle().connect([this]() {
			auto adjustment = m_text_window->get_vadjustment();
			adjustment->set_value(adjustment->get_upper());

			return false;
		});

		return false;
	});
}

