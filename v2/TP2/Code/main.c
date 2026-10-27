// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "token.h"
#include "queue.h"
#include "stack.h"

#define DEBUG_TAD 0

/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);

/** 
 * Diplay on stdout the operation 'pop' of the TAD with arrows left
 */
void debug_pop(const void* e, char* s) {
	if (DEBUG_TAD==1) {
		if(token_is_number(e)) printf("%f <-- %s\n", token_value(e), s);
		if(token_is_operator(e)) printf("%c <-- %s\n", token_operator(e), s);
		if(token_is_parenthesis(e)) printf("%c <-- %s\n", token_parenthesis(e), s);
	}
}

/** 
 * Display on stdout the operation 'push' of the TAD with right
 */
void debug_push(const void* e, char* s) {
	if (DEBUG_TAD==1) {
		if(token_is_number(e)) printf("%f --> %s\n", token_value(e), s);
		if(token_is_operator(e)) printf("%c --> %s\n", token_operator(e), s);
		if(token_is_parenthesis(e)) printf("%c --> %s\n", token_parenthesis(e), s);
	}
}

/** 
 * Test function
 */
bool isSymbol(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '(' || c == ')';
}

Token* evaluateOperator(Token* arg1, Token* op, Token* arg2) {
	float op1 = token_value(arg1), op2 = token_value(arg2), res;
	switch (token_operator(op))
	{
	case '+':
		res = op1+op2;
		break;
	case '-':
		res = op1-op2;
		break;
	case '/':
		assert(op2!=0);
		res = op1/op2;
		break;
	case '^':
		res = powf(op1,op2);
		break;
	case '*':
		res = op1*op2;
		break;
	default:
		res = 0;
		break;
	}
	return create_token_from_value(res);
}

/** 
 *  Conversion of an expresion to a queue of token.
 */
Queue* stringToTokenQueue(const char* expression) {
	Queue* queue = create_queue();
	Token* t;
	const char* curpos = expression;
	const char* number_curpos;
	int nb_digit = 1;
	while (*curpos!='\0') {
		while (*curpos!=' ' && *curpos!='\n') {
			if(!isSymbol(*curpos)) {
				number_curpos = curpos;
				number_curpos++;
				while (*number_curpos!=' ' && *number_curpos!='\n' && *number_curpos!='\0' && !isSymbol(*number_curpos)) {
					nb_digit++;
					number_curpos++;
				}
			}
			t = create_token_from_string(curpos, nb_digit);
			queue = queue_push(queue, t);
			curpos++;
			if(nb_digit>1) for (int j = 0; j<nb_digit-1; j++) curpos++;
			nb_digit = 1;
		}
		curpos++;
	}
	return queue;
}

/** 
 *  Conversion of an infix queue of token in a postfix queue of token.
 */
Queue* shuntingYard(Queue* infix) {
	Queue* output = create_queue();
	Stack* operator = create_stack(queue_size(infix));
	const Token* token;
	const Token* token_stack;
	while (!queue_empty(infix)) {
		token = queue_top(infix);
		if (token_is_number(token)) {
			debug_push(token,"output");
			output = queue_push(output, token);
		}
		if (token_is_operator(token)) {
			while (!stack_empty(operator) && (token_is_operator(stack_top(operator))
			&& ((token_operator_priority(stack_top(operator))>token_operator_priority(token))
			|| ((token_operator_priority(stack_top(operator))==token_operator_priority(token)) 
			&& token_operator_leftAssociative(token)))) 
			&& (!token_is_parenthesis(stack_top(operator)) || (token_is_parenthesis(stack_top(operator)) && (token_operator(stack_top(operator))!='(')))) {

				token_stack = stack_top(operator);

				debug_pop(token_stack, "operator");
				operator = stack_pop(operator);

				debug_push(token_stack, "output");
				output = queue_push(output, token_stack);
			}
			debug_push(token, "operator");
			operator = stack_push(operator, token);
		}
		if (token_is_parenthesis(token) && token_parenthesis(token)=='(') {
			debug_push(token, "operator");
			operator = stack_push(operator, token);
		}
		if (token_is_parenthesis(token) && token_parenthesis(token)==')') {
			while (!stack_empty(operator) && (!token_is_parenthesis(stack_top(operator)) || token_parenthesis(stack_top(operator))==')')) {
				token_stack = stack_top(operator);
				
				debug_pop(token_stack, "operator");
				operator = stack_pop(operator);
				
				debug_push(token_stack, "output");
				output = queue_push(output, token_stack);
			}
			if (!stack_empty(operator) && token_is_parenthesis(stack_top(operator)) && (token_parenthesis(stack_top(operator))=='(')) {
				Token* parenthesis1 = (Token*)stack_top(operator);
				debug_pop(stack_top(operator), "operator");
				operator = stack_pop(operator);
				delete_token(&parenthesis1);
			}

			Token* parenthesis2 = (Token*) token;
			debug_pop(token, "infix");
			infix = queue_pop(infix);
			delete_token(&parenthesis2);
			continue;
		}
		debug_pop(token, "infix");
		infix = queue_pop(infix);
	}
	if (queue_empty(infix)) {
		while (!stack_empty(operator)) {
			token_stack = stack_top(operator);
			
			debug_pop(token_stack, "operator");
			operator = stack_pop(operator);

			if (token_is_parenthesis(token_stack)) {
				Token* t = (Token*)token_stack;
				delete_token(&t);
			} else {
				debug_push(token_stack, "output");
				output = queue_push(output, token_stack);
			}
		}
	}
	if(stack_empty(operator)) delete_stack(&operator);
	delete_queue(&infix);
	return output;
}

/**
 * Evaluate the expression with the postfix queue of token.
 */
float evaluateExpression(Queue* postfix) {
	const Token* token;
	const Token* op1;
	const Token* op2;
	const Token* result;
	float res;
	Stack* stack = create_stack(queue_size(postfix));
	while (!queue_empty(postfix)) {
		token = queue_top(postfix);
		if (token_is_operator(token)) {
			op2 = stack_top(stack);
			stack = stack_pop(stack);
			op1 = stack_top(stack);
			stack = stack_pop(stack);
			result = evaluateOperator((Token*)op1, (Token*)token, (Token*)op2);
			stack = stack_push(stack, result);
			Token* temp = (Token*)op1;
			delete_token(&temp);
			temp = (Token*)op2;
			delete_token(&temp);
			temp = (Token*)token;
			delete_token(&temp);
		} 
		else if(token_is_number(token)) {
			if (!stack_overflow(stack)) stack = stack_push(stack, token);
		}
		postfix = queue_pop(postfix);
	}
	result = stack_top(stack);
	(!stack_empty(stack) && token_is_number(stack_top(stack)))?(res = token_value(result)):(res = 0);
	Token* result_d = (Token*)result; 
	delete_token(&result_d);
	delete_stack(&stack);
	return res;
}

/** 
 * Display of differents arithmétic notations in stdout.
 */
void computeExpressions(FILE* input) {
	char* line = NULL;
	size_t n;
	Queue* q;
	while (getline(&line, &n,input)!=EOF) {
		if( line != NULL && line[0]!='\n') {
			line[n-1]= '\0';
			printf("Input    : %s", line);
			q = stringToTokenQueue(line);
			printf("Infix    : ");
			print_queue(stdout, q);
			printf("\n");
			q = shuntingYard(q);
			printf("Postfix  : ");
			print_queue(stdout, q);
			printf("\n");
			printf("Evaluate : %f\n", evaluateExpression(q));
			printf("\n");
			
			delete_queue(&q);
		}
	}
	free(line);
}

/** Main function for testing.
 * The main function expects one parameter that is the file where expressions to translate are
 * to be read.
 *
 * This file must contain a valid expression on each line
 *
 */
int main(int argc, char** argv) {
	if (argc<2) {
		fprintf(stderr,"usage : %s filename\n", argv[0]);
		return 1;
	}
	
	FILE* input = fopen(argv[1], "r");

	if ( !input ) {
		perror(argv[1]);
		return 1;
	}

	computeExpressions(input);

	fclose(input);
	return 0;
}
 
void print_token(const void* e, void* user_param) {
	FILE* f = (FILE*)user_param;
	Token* t = (Token*)e;
	token_dump(f, t);
}

void print_queue(FILE* f, Queue* q) {
	fprintf(f, "(%d) --  ", queue_size(q));
	queue_map(q, print_token, f);
}