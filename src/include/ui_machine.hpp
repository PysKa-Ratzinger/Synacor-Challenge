#ifndef UI_MACHINE_HPP
#define UI_MACHINE_HPP

#include <gtkmm.h>

class UiMachine : public Gtk::ApplicationWindow {
public:
	UiMachine();
	~UiMachine();

	void run_program();

private:
	Glib::RefPtr<Gtk::Builder> m_builder;
};

#endif  // UI_MACHINE_HPP

