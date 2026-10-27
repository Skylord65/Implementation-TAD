// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "token.h"
#include "../Lib/queue.h"
#include "../Lib/stack.h"


/** 
 * Utilities functions
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);
Queue* stringToTokenQueue(const char* expression);
Queue* shuntingYard(Queue* infix);
float evaluateExpression(Queue* postfix);
bool isSymbol(char c);
Token* evaluateOperator(Token* arg1, Token* op, Token* arg2);
/** 
 * Function to be written by students
 */
void computeExpressions(FILE* input) {
	char *line = NULL;
	size_t n;
	Queue* tq;
	Queue* p;
	while (getline(&line, &n, input)!=EOF) {
		if (strcmp(line,"\n")) {
			printf("Input    : %s", line);
			tq = stringToTokenQueue(line);
			printf("Infix    : ");
			print_queue(stdout, tq);
			printf("\n");
			p = shuntingYard(tq);
			printf("Postfix  : ");
			print_queue(stdout, p);
			printf("\nEvaluate : %f\n\n", evaluateExpression(p));
			delete_queue(&tq);
			
		}
	}
	free(line);
}

Queue* stringToTokenQueue(const char* expression) {
	Queue *q = create_queue();
	const char *curpos = expression;
	const char* curpos_debut = curpos;
	int i;
	while (*curpos!='\0') {
		i = 0;
		while ((*curpos!='\0')&&((*curpos==' ')||(*curpos=='\n'))) curpos++;
		curpos_debut = curpos;
		if ((!isSymbol(*curpos))&&(*curpos!=' ')&&(*curpos!='\n')&&(*curpos!='\0')) {
			i++;
			curpos++;
			while ((!isSymbol(*curpos))&&(*curpos!=' ')&&(*curpos!='\n')&&(*curpos!='\0')) {
				i++;
				curpos++;
			}
		} else {
			if (*curpos!='\0') curpos++;
			i++;
		}
		if (*curpos_debut!='\0') queue_push(q, create_token_from_string(curpos_debut, i));
	}
	return q;
}

Queue* shuntingYard(Queue* infix) {
	const Token* token;
	const Token* top_operator;
	Token* to_delete;
	Queue* postfix = create_queue();
	Stack* operator_stack = create_stack(queue_size(infix));
	while (!queue_empty(infix)) {
		token = queue_top(infix);
		queue_pop(infix);
		if (token_is_number(token)) queue_push(postfix, token);
		if (token_is_operator(token)) {
			if (!stack_empty(operator_stack)) top_operator =  stack_top(operator_stack);
			while ((!stack_empty(operator_stack) && token_is_operator(top_operator)) 
					&& ((( token_operator_priority(token) < token_operator_priority(top_operator)) 
					|| (token_operator_priority(token) == token_operator_priority(top_operator) && token_operator_leftAssociative(token))) 
					&& !(token_is_parenthesis(top_operator) && token_parenthesis(top_operator)=='('))) {
				stack_pop(operator_stack);
				queue_push(postfix, top_operator);
				if (!stack_empty(operator_stack)) top_operator =  stack_top(operator_stack);
			}
			stack_push(operator_stack, token);
		}
		if (token_is_parenthesis(token) && token_parenthesis(token)=='(') {
			stack_push(operator_stack, token);
		}
		if (token_is_parenthesis(token) && token_parenthesis(token)==')') {
			if (!stack_empty(operator_stack)) top_operator =  stack_top(operator_stack);
			while (!stack_empty(operator_stack) && !(token_is_parenthesis(top_operator) && token_parenthesis(top_operator)=='(')) {
				stack_pop(operator_stack);
				queue_push(postfix, top_operator);
				if (!stack_empty(operator_stack)) top_operator =  stack_top(operator_stack);
			}
			to_delete = (Token*) stack_top(operator_stack);
			stack_pop(operator_stack);
			delete_token(&to_delete);
			to_delete = (Token*)token;
			delete_token(&to_delete);
		}
	}
	
	if(queue_empty(infix)) {
		if (!stack_empty(operator_stack)) top_operator =  stack_top(operator_stack);
		while (!stack_empty(operator_stack)) {
			stack_pop(operator_stack);
			queue_push(postfix, top_operator);
			if (!stack_empty(operator_stack)) top_operator =  stack_top(operator_stack);
		}
	}

	delete_stack(&operator_stack);

	return postfix;
}

float evaluateExpression(Queue* postfix) {
	const Token* top_token;
	Token* op1;
	Token* op2;
	Token* token_to_delete;
	Stack* stack = create_stack(queue_size(postfix));
	float res;
	while (!queue_empty(postfix)) {
		top_token = queue_top(postfix);
		queue_pop(postfix);
		if (token_is_operator(top_token)) {
			op2 = (Token*)stack_top(stack);
			stack_pop(stack);
			op1 = (Token*)stack_top(stack);
			stack_pop(stack);
			stack_push(stack, (const Token*)evaluateOperator(op1,(Token*)top_token, op2));
			delete_token(&op1);
			delete_token(&op2);
			token_to_delete = (Token*) top_token;
			delete_token(&token_to_delete);
		} else if (token_is_number(top_token)) {
			stack_push(stack, top_token);
		}
	}
	token_to_delete = (Token*)stack_top(stack);
	stack_pop(stack);
	res = token_value(token_to_delete);
	delete_token(&token_to_delete);
	delete_stack(&stack);
	delete_queue(&postfix);
	return res;
}

/** Main function for testing.
 * The main function expects one parameter that is the file where expressions to translate are
 * to be read.
 *
 * This file must contain a valid expression on each line
 *
 */
int main(int argc, char** argv){
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

bool isSymbol(char c) {
	return c=='+'||c=='-'||c=='*'||c=='/'||c=='^'||c=='('||c==')';
}

Token* evaluateOperator(Token* arg1, Token* op, Token* arg2) {
	assert(token_is_operator(op) && token_is_number(arg1) && token_is_number(arg2));
	float v1 = token_value(arg1), v2 = token_value(arg2), res;
	switch (token_operator(op)) {
	case '+':
		res=v1+v2;
		break;
	case '-':
		res=v1-v2;
		break;
	case '*':
		res=v1*v2;
		break;
	case '/':
		assert(v2!=0);
		res=v1/v2;
		break;
	case '^':
		res=powf(v1, v2);
		break;
	default:
		perror("Wrong Opérator");
		exit(1);
	}
	return create_token_from_value(res);
}