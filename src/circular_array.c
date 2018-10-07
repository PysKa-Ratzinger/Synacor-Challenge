#include "circular_array.h"

#include <stdio.h>
#include <string.h>

struct circular_array_16* circular_array_create(size_t size) {
	struct circular_array_16* res = malloc(sizeof(*res));
	if (!res) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	memset(res, 0, sizeof(*res));
	res->max_size = size;

	res->array = malloc(sizeof(uint16_t) * size);
	if (!res->array) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	return res;
}

void circular_array_free(struct circular_array_16* arr) {
	free(arr->array);
	free(arr);
}

void circular_array_print(struct circular_array_16* arr) {
	size_t prints = 0;
	size_t index;
	if (arr->curr_size < arr->max_size) {
		index = 0;
	} else {
		index = arr->array_tail;
	}

	while (prints < arr->curr_size) {
		printf("%04x, ", arr->array[index]);
		prints++;
		index = (index + 1) % arr->max_size;
		if (prints % 15 == 0) {
			printf("\n");
		}
	}
	if (prints % 15 != 0) {
		printf("\n");
	}
}

void circular_array_insert(struct circular_array_16* arr, uint16_t elem) {
	uint16_t index = arr->array_tail;
	arr->array[index] = elem;
	if (arr->curr_size < arr->max_size) {
		arr->curr_size++;
	}
	arr->array_tail = (arr->array_tail + 1) % arr->max_size;
}

