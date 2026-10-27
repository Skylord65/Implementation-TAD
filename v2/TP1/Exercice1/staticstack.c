#include "stack.h"
#include <assert.h>
#include <stdlib.h>
#define STACK_SIZE 16

/** Full definition of the s_stack structure */
struct s_stack {
	int stack[STACK_SIZE];
	int top;
};

/** Create and initialize a stack. */
Stack* stack(void) {
	Stack* s = malloc(sizeof(struct s_stack));
	s->top=-1;
	return (s);
}

/** Push a value on the stack
 * @pre push(s,e) defined if s->top < STACK_SIZE
 */
Stack* push(Stack* s, int e) {
	assert(s != NULL);
	assert(s->top < STACK_SIZE);
	s->stack[++(s->top)] = e;
	return(s);
}

/** Test if a stack is empty.
 */
bool empty(const Stack* s) {
	assert(s != NULL);
	return (s->top == -1);
}

/** Pop a value from the stack
 * @pre pop(s) defined if \f$\neg\f$ empty(s) */
Stack* pop(Stack* s) {
	assert(s != NULL);
	assert(!empty(s));
	--(s->top);
	return(s);
}

/** Return the top of a stack
 * @pre top(s) defined if \f$\neg\f$ empty(s)
 */
int top(const Stack* s) {
	assert(s != NULL);
	assert(!empty(s));
	return (s->stack[s->top]);
}

/** Return true if the stack will overflow on the next push. */
bool overflow(const Stack* s){
	assert(s != NULL);
	return s->top + 1 == STACK_SIZE;
}

/** Dump the stack (print it) on the given FILE* */
void dump(const Stack* s, FILE* f){
	assert(s != NULL);
	fprintf(f, "stack size = %d\n", s->top+1);
	for (int i=s->top; i>=0; --i)
		fprintf(f, "\t%d\n", s->stack[i]);
}
