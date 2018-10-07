#pragma once

#include <stdint.h>
#include <stdlib.h>

struct circular_array_16 {
	uint16_t* array;
	size_t max_size;
	size_t curr_size;
	size_t array_tail;
};

struct circular_array_16* circular_array_create(size_t size);

void circular_array_free(struct circular_array_16* arr);

void circular_array_print(struct circular_array_16* arr);

void circular_array_insert(struct circular_array_16* arr, uint16_t elem);

