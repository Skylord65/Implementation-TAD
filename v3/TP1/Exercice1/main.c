#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

int readInt(FILE* input) {
  int v;
  int r = fscanf(input,"%d", &v);
  if (r == 1) {
    return v;
  }
  perror("Unable to read int from input file\n");
  abort();
}

/** Main function for testing the Stack implementation.
 * The main function expects one parameter that is the file where values added to the stack are
 * to be read.
 *
 * This file must contain the following information :
 * - on the first line, the number of values to be added to the stack,
 * - on the second line, the values to be added, separated by a space (or tab).
 *
 * The values will be added to the stack in the order they are read from the file.
 */
int main(int argc, char **argv){
	if (argc<2) {
		fprintf(stderr,"usage : %s filename\n", argv[0]);
		return 1;
	}

	FILE* input = fopen(argv[1], "r");
	if ( !input ) {
		perror(argv[1]);
		return 1;
	}

	Stack* theStack = stack();
        int n = readInt(input);
	for ( int i=0; i<n; ++i ) {
		int v = readInt(input);
		theStack = push(theStack, v);
	}

	dump(theStack,stderr);

	while (!empty(theStack)) {
		printf ("Removing element %d\n", top(theStack));
		theStack = pop(theStack);
	}
	return 0;
}

