#ifndef STACK_H_
#define STACK_H_

#include <iostream>
#include <stack>

/*
 * Prints the stack
 */
template <typename T>
void stack_print(const std::stack<T>& q)
{
	std::stack<T> tmp = q;

	std::cout << "STACK BASE" << std::endl;
	while (tmp.size()) {
		T val = tmp.top();
		std::cout << ": " << val << std::endl;
		tmp.pop();
	}
	std::cout << "STACK STOP" << std::endl;
}

/*
 * Prints to stdout the result of comparing two stacks
 */
template <typename T>
void stack_show_compare(const std::stack<T>& q1, const std::stack<T>& q2)
{
	std::stack<T> t1 = q1;
	std::stack<T> t2 = q2;

	std::cout << "---- STACK COMPARISON START ----" << std::endl;

	T val1, val2;
	size_t pos = 0;
	size_t n_equal = 0;
	while (t1.size()) {
		val1 = t1.top();
		t1.pop();
		pos++;

		if (t2.size()) {
			val2 = t2.top();
			t2.pop();

			if (val1 == val2) {
				n_equal++;
			} else {
				if (n_equal) {
					std::cout << "..." << std::endl;
					std::cout << n_equal
						<< "equal elements" << std::endl;
					std::cout << "..." << std::endl;
					n_equal = 0;
				}
				std::cout << pos << ": " << val1 << " - "
					<< val2 << std::endl;
			}
		} else {
			std::cout << pos << ": " << val1 << " - ..."
				<< std::endl;
		}
	}

	if (n_equal) {
		std::cout << "..." << std::endl;
		std::cout << n_equal
			<< "equal elements" << std::endl;
		std::cout << "..." << std::endl;
	}

	while (t1.size()) {
		val1 = t1.top();
		t1.pop();
		pos++;
		std::cout << pos << ": " << val1 << " - ..." << std::endl;
	}

	while (t2.size()) {
		val2 = t2.top();
		t2.pop();
		pos++;
		std::cout << pos << ": ... - " << val2 << std::endl;
	}

	std::cout << "---- STACK COMPARISON END ----" << std::endl;
}

#endif  // STACK_H_

