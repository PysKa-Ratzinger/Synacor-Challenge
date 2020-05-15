#ifndef UI_MACHINE_HPP
#define UI_MACHINE_HPP

#include <gtkmm.h>

#include "ctrl/ui_machine_ctrl.hpp"

class UiMachine : public Gtk::ApplicationWindow {
public:
	UiMachine();
	~UiMachine();

	void run_program();
	void load_program();
	void stop_running();

private:
	void key_pressed(GdkEventKey* event);
	void handle_output(int type, const char* output);

	Glib::RefPtr<Gtk::Builder> m_builder;
	Glib::RefPtr<Gtk::TextView> m_text_window;
	Glib::RefPtr<Gtk::Entry> m_user_input;

	MachineController m_ctrl;
};

#endif  // UI_MACHINE_HPP

