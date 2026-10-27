#include "stack.h"
#include <assert.h>
#include <stdlib.h>

/** Linked list representation of a stack
 */
typedef struct s_element {
	int value;
	struct s_element* next;
} Element;

/** Internal representation of the TAD Stack
 */
struct s_stack {	
	Element* top;
	size_t size;
};

/** Create and initialize a stack. */
Stack* stack(void) {
	Stack* s = malloc(sizeof(struct s_stack));
	s->top=NULL;
	s->size = 0;
	return s;
}

/** Push a value on the stack
 * @pre push(s,e) defined if s->top < STACK_SIZE
 */
Stack* push(Stack* s, int e) {
	assert(s != NULL);
	Element* new = 0;
	new->value = e;
	new->next = s->top;
	s->top = new;
	++s->size;
	return s;
}

/** Test if a stack is empty.
 */
bool empty(const Stack* s) {
	assert(s != NULL);
	return s->top == NULL;
}

/** Pop a value from the stack
 * @pre pop(s) defined if \f$\neg\f$ empty(s) */
Stack* pop(Stack* s) {
	assert(s != NULL);
	assert(!empty(s));
	Element* e = s->top;
	s->top = e->next;
	free(e);
	--s->size;
	return s;
}

/** Return the top of a stack
 * @pre top(s) defined if \f$\neg\f$ empty(s)
 */
int top(const Stack* s) {
	assert(s != NULL);
	assert(!empty(s));
	return s->top->value;
}

/** Return true if the stack will overflow on the next push. */
bool overflow(const Stack* s){
	assert(s != NULL);
	(void)s;
	return false;
}

/** Dump the stack (print it) on the given FILE* */
void dump(const Stack* s, FILE* f){
	assert(s != NULL);
	fprintf(f, "stack size = %lu\n", s->size);
	for (Element* i=s->top; i!=NULL; i = i->next)
		fprintf(f, "%d ", i->value);
	fprintf(f, "\n");
}
