#include "stack.h"

#include <stdio.h>

/*
 * struct stack16_cell: A single cell capable of holding 32 16-bit values.
 *
 * This cell is a smaller stack which can be stacked on top of other cells.
 * It was designed to be more memory efficient than storing a single value
 * per cell, which would result in 2 (value) / 10 (pointer + value) =
 * 20% efficiency, where in this case it results in 510 (2*255 values) /
 * 520 (pointer + values + num_vals) = 98% efficiency.
 */
struct stack16_cell;

struct stack16_cell* stack16_cell_create(struct stack16_cell* prev);
void stack16_cell_free(struct stack16_cell* c);
void stack16_cell_free_recursive(struct stack16_cell* c);
struct stack16_cell* stack16_cell_push(struct stack16_cell* cell,
                                       uint16_t value);
struct stack16_cell* stack16_cell_pop(struct stack16_cell* cell,
                                      uint16_t* val);

/*
 * struct stack16: Structure representing a full stack which holds individual
 * 16-bit values
 */
struct stack16 {
    /*
     * The top of the stack, containing the last inserted stack cell, if
     * there is any.
     */
    struct stack16_cell* top;

    /*
     * A value containing the number of elements currently in the stack.
     */
    size_t num_elems;
};

struct stack16_cell {
    /*
     * Smaller stack of 255 values (not 256 in order to align memory with
     * num_vals.
     */
    uint16_t vals[255];

    /*
     * Contains the number of values currently stored by this cell
     */
    uint16_t num_vals;

    /*
     * Pointer to the previous cell in the stack
     */
    struct stack16_cell* prev;
};

struct stack16_cell* stack16_cell_create(struct stack16_cell* prev) {
    struct stack16_cell* res = NULL;
    res = (struct stack16_cell*) malloc(sizeof(struct stack16_cell));
    if (res == NULL)
        return NULL;
    memset(res->vals, 0, sizeof(res->vals));
    res->num_vals = 0;
    res->prev = prev;
    return res;
}

void stack16_cell_free(struct stack16_cell* c) {
    if (c == NULL)
        return;
    return free(c);
}

void stack16_cell_free_recursive(struct stack16_cell* c) {
    struct stack16_cell* curr = c;
    struct stack16_cell* prev = NULL;
    while (curr) {
        prev = curr->prev;
        free(curr);
        curr = prev;
    }
}

struct stack16* stack16_create() {
    struct stack16* res = NULL;
    res = (struct stack16*) malloc(sizeof(struct stack16));
    if (res != NULL) {
        memset(res, 0, sizeof(struct stack16));
    }
    return res;
}

void stack16_free(struct stack16* s) {
    if (s->num_elems) {
        stack16_cell_free_recursive(s->top);
    }
    return free(s);
}

struct stack16_cell* stack16_cell_push(struct stack16_cell* cell,
                                       uint16_t value) {
    if (cell == NULL)
        return NULL;
    struct stack16_cell* new_top = cell;
    if (cell->num_vals == (sizeof(cell->vals) / sizeof(uint16_t))) {
        new_top = stack16_cell_create(cell);
        if (new_top == NULL)
            return NULL;
    }
    new_top->vals[new_top->num_vals] = value;
    new_top->num_vals++;
    return new_top;
}

int stack16_push(struct stack16* s, uint16_t value) {
    if (s == NULL)
        return 1;
    if (s->num_elems == 0) {
        struct stack16_cell* base = stack16_cell_create(NULL);
        s->top = base;
        s->num_elems++;
    }
    struct stack16_cell* new_top = stack16_cell_push(s->top, value);
    if (new_top == NULL)
        return 2;
    if (s->top != new_top) {
        s->num_elems++;
        s->top = new_top;
    }
    return 0;
}

struct stack16_cell* stack16_cell_pop(struct stack16_cell* cell,
                                      uint16_t* val) {
    if (cell == NULL)
        return NULL;
    cell->num_vals--;
    *val = cell->vals[cell->num_vals];
    if (cell->num_vals == 0) {
        struct stack16_cell* res = cell->prev;
        stack16_cell_free(cell);
        return res;
    }
    return cell;
}

int stack16_pop(struct stack16* s, uint16_t* val) {
    if (s == NULL || val == NULL || s->num_elems == 0)
        return 1;
    struct stack16_cell* new_top = stack16_cell_pop(s->top, val);
    if (new_top != s->top) {
        s->num_elems--;
        s->top = new_top;
    }
    return 0;
}

void stack16_print(struct stack16* s) {
	struct stack16* tmp = stack16_create();
	uint16_t val;
	while (stack16_pop(s, &val) == 0) {
		stack16_push(tmp, val);
	}
	printf("STACK BASE\n");
	while (stack16_pop(tmp, &val) == 0) {
		printf(": %04x\n", val);
		stack16_push(s, val);
	}
	printf("STACK TOP\n");
	stack16_free(tmp);
}

struct stack16* stack_duplicate(struct stack16* stack) {
	struct stack16* tmp = stack16_create();
	struct stack16* res = stack16_create();
	uint16_t val;
	while (stack16_pop(stack, &val) == 0) {
		stack16_push(tmp, val);
	}
	while (stack16_pop(tmp, &val) == 0) {
		stack16_push(stack, val);
		stack16_push(res, val);
	}
	stack16_free(tmp);
	return res;
}

