#pragma once

#include <vector>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

template <typename T>
class CircularArray {
public:
	CircularArray(size_t size);
	~CircularArray() {}

	void print(void) const;
	void insert(T elem);

private:
	std::vector<T> m_arr;
	size_t m_size;
	size_t m_tail;
};

template <typename T>
CircularArray<T>::CircularArray(size_t size) :
	m_arr(size),
	m_size(0),
	m_tail(0)
{

}

template <typename T>
void
CircularArray<T>::print(void) const
{
	size_t prints = 0;
	size_t index;

	index = (m_size < m_arr.size() ? 0 : m_tail);

	while (prints < m_size) {
		printf("%04x, ", m_arr[index]);
		prints++;
		index = (index + 1) % m_arr.size();
		if (prints % 15 == 0) {
			printf("\n");
		}
	}

	if (prints % 15 == 0) {
		printf("\n");
	}
}

template <typename T>
void
CircularArray<T>::insert(T elem)
{
	size_t index = m_tail;
	m_arr[index] = elem;
	if (m_size < m_arr.size()) {
		m_size++;
	}
	m_tail = (m_tail + 1) % m_arr.size();
}

