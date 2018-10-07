#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct stack16;

/*
 * Creates an initialized stack structure
 */
struct stack16* stack16_create();

/*
 * Frees the memory allocated for the stack
 */
void stack16_free(struct stack16* s);

/*
 * Pushes a 16-bit value into the stack
 */
int stack16_push(struct stack16* s, uint16_t value);

/*
 * Pops a 16-bit value from the stack
 */
int stack16_pop(struct stack16* s, uint16_t* val);

/*
 * Prints the stack
 */
void stack16_print(struct stack16* s);

#endif  // STACK_H_

