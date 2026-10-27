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


/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);
void print_stack(FILE* f,Stack* s);
Queue* stringToTokenQueue(const char* expression);
Queue* shuntingYard(Queue* infix);
float evaluateExpression(Queue* postfix);
Token* evaluateOperator(Token* arg1, Token* op, Token* arg2);

/** 
 * Function to be written by students
 */
void computeExpressions(FILE* input) {
	char* line = NULL;
	size_t n = 0;
	while (getline(&line, &n, input)!=EOF)
	{
		if (strcmp(line, "\n")!=0){
			// Input
			printf("Input : %s", line);

			// Infix
			printf("Infix : ");
			Queue *infix_queue = stringToTokenQueue(line);
			print_queue(stdout, infix_queue);
			printf("\n");
			
			// Postfix
			Queue *postfix_queue = shuntingYard(infix_queue);
			printf("Postfix : ");
			print_queue(stdout, postfix_queue);
			printf("\n");
			
			// Evalute expression

			float evaluate = evaluateExpression(postfix_queue);
			printf("Evaluate : %f", evaluate);
			printf("\n");

			/*
			// Free Infix mis en commentaire car il sera vide après postfix

			for (;!queue_empty(infix_queue);){
				Token* token_temp = (Token*)queue_top(infix_queue);
				infix_queue = queue_pop(infix_queue);
				delete_token(&token_temp);
			}
			
			infix_queue = NULL;
			*/
		
			// Free Postfix
			
			for (;!queue_empty(postfix_queue);){
				Token* token_temp = (Token*)queue_top(postfix_queue);
				postfix_queue = queue_pop(postfix_queue);
				delete_token(&token_temp);
			}
			delete_queue(&infix_queue);
			delete_queue(&postfix_queue);
			postfix_queue = NULL;
			
		}
	}

	free(line);
}

/** 
 * Function to verify if a char is a symbol +, -, /, ^, ( or ).
 */
bool isSymbol(char c){
	return c=='+'||c=='-'||c=='*'||c=='/'||c=='^'||c=='('||c==')';
}

/** 
 * Function to change an expression to a Queue of Token.
 */
Queue* stringToTokenQueue(const char* expression){
	Queue* q = create_queue();
	const char* curpos = expression;
	int i=0;
	const char* debut;
	//debut = curpos;
	while (*curpos!='\0')
	{	

		while (*curpos==' ' || *curpos=='\n')
		{
			i = 0;
			curpos++;
		}
		debut = curpos;
		if (!isSymbol(*curpos)){
			curpos++;
			i++;
			while (!isSymbol(*curpos) && *curpos!=' ' && *curpos!='\n' && *curpos!='\0')
			{
				i++;
				curpos++;
			}	
		} else{
			i++;
			curpos++;
		}
		if(*debut!='\0' && *debut!='\n' && i!=0){
			Token* Elem_syntaxique = create_token_from_string(debut,i);
			q = queue_push(q,Elem_syntaxique);
			//delete_token(&Elem_syntaxique);
		}	
	}
	
	return q;
}

/** 
 * Function to change Infix Queue to a Postfix Queue of Token.
 */
Queue* shuntingYard(Queue* infix){
	Queue* postfix = create_queue();
	Stack* operator_stack = create_stack(32);
	//memcpy(postfix, infix, queue_size(infix));
	
	while (!queue_empty(infix)){
		Token *element = (Token*)queue_top(infix);
		if (token_is_number(element)){
			//printf("element : %f and this is a number\n", token_value(element));
			postfix = queue_push(postfix, element);

		}
		if (token_is_operator(element)){
			//printf("element : %c and this is an operator\n", token_operator(element));
			Token* top_operator = NULL;
			if(!stack_empty(operator_stack)){
				top_operator = (Token*)stack_top(operator_stack);
			}
			while ((!stack_empty(operator_stack)) && ((token_is_operator(element) && token_is_operator(top_operator) && ((token_operator_priority(element)<token_operator_priority(top_operator)) || ((token_operator_priority(element)==token_operator_priority(top_operator)) && token_operator_leftAssociative(element)))) && (!token_is_parenthesis(top_operator) || (token_is_parenthesis(top_operator) && token_parenthesis(top_operator)!='(')))) 
			{	
				//token_is_operator(top_operator) ? printf("retrait stack : %c", token_operator(top_operator)):printf("retrait stack : %c", token_parenthesis(top_operator));
				operator_stack = stack_pop(operator_stack);
				postfix = queue_push(postfix, top_operator);
				if(!stack_empty(operator_stack)){
					top_operator = (Token*)stack_top(operator_stack);
				}
			}
			operator_stack = stack_push(operator_stack, element);
		}
		if (token_is_parenthesis(element) && token_parenthesis(element)=='('){
			//printf("element : %c and this is a parenthesis\n", token_parenthesis(element));
			operator_stack = stack_push(operator_stack, element);
		}
		if (token_is_parenthesis(element) && token_parenthesis(element)==')'){
			//printf("element : %c and this is a parenthesis\n", token_parenthesis(element));
			Token* top_operator = NULL;
			if(!stack_empty(operator_stack)){
				top_operator = (Token*)stack_top(operator_stack);
			}
			while (!stack_empty(operator_stack) && (!token_is_parenthesis(top_operator) || (token_is_parenthesis(top_operator) && token_parenthesis(top_operator)!='(')))
			{
				operator_stack = stack_pop(operator_stack);
				postfix = queue_push(postfix, top_operator);
				if(!stack_empty(operator_stack)){
					top_operator = (Token*)stack_top(operator_stack);
				}
			}
			if(!stack_empty(operator_stack)){
				operator_stack = stack_pop(operator_stack);
				delete_token(&top_operator);
				delete_token(&element);
			}
		}

		//print_queue(stdout, postfix);
		//printf("\n");
		//print_stack(stdout, operator_stack);
		//printf("\n");
		
		infix = queue_pop(infix);
	}
	Token* top_operator = NULL;
	if(!stack_empty(operator_stack)){
		top_operator = (Token*)stack_top(operator_stack);
	}
	while (!stack_empty(operator_stack))
	{
		operator_stack = stack_pop(operator_stack);
		postfix = queue_push(postfix, top_operator);
		if(!stack_empty(operator_stack)){
			top_operator = (Token*)stack_top(operator_stack);
		}
	}
	// Empty and free the stack
	if (stack_empty(operator_stack))
	{
		delete_stack(&operator_stack);
	}
	else
	{
		while (!stack_empty(operator_stack))
		{
			Token* temp_token = (Token*)stack_top(operator_stack);
			operator_stack = stack_pop(operator_stack);
			delete_token(&temp_token);
		}
		delete_stack(&operator_stack);
	}

	return postfix;
}

/** 
 * Function to change a postfic Queue of Token to its value.
 */
float evaluateExpression(Queue* postfix){
	Stack* operand_stack = create_stack(queue_size(postfix));
	Token* operand_1;
	Token* operand_2;
	Token* result;
	for (;!queue_empty(postfix);){
		Token* operator = (Token*)queue_top(postfix);
		if (token_is_operator(operator)){

			// Fill the operand
			operand_2 = (Token*)stack_top(operand_stack);
			operand_stack = stack_pop(operand_stack);
			operand_1 = (Token*)stack_top(operand_stack);
			operand_stack = stack_pop(operand_stack);

			// Evaluate
			result = evaluateOperator(operand_1, operator,operand_2);

			// Free token
			delete_token(&operand_1);
			delete_token(&operand_2);
			delete_token(&operator);

			// push the stack
			operand_stack = stack_push(operand_stack, result);

		} else if (token_is_number(operator))
		{
			operand_stack = stack_push(operand_stack, operator);
		}

		//print_stack(stdout, operand_stack);
		//printf("\n");

		postfix = queue_pop(postfix);
	}

	// return result from the top of the stack
	result = (Token*)stack_top(operand_stack);
	operand_stack = stack_pop(operand_stack);
	delete_stack(&operand_stack); // Free the stack
	float result_expression = token_value(result);
	delete_token(&result);
	return result_expression;
}

/** 
 * Function to evaluate values with the good operator.
 */
Token* evaluateOperator(Token* arg1, Token* op, Token* arg2){
	float result = 0;
	if(token_is_number(arg1) && token_is_number(arg2)){
		float operand_1 = token_value(arg1);
		float operand_2 = token_value(arg2);
		switch (token_operator(op))
		{
		case '+':
			result = operand_1 + operand_2;
			break;
		case '-':
			result = operand_1 - operand_2;
			break;
		case '*':
			result = operand_1 * operand_2;
			break;
		case '/':
			if (operand_2==0){
				printf("Division par 0 pas possible");
				exit(1);
			}
			result = operand_1 / operand_2;
			break;
		case '^':
			result = powf(operand_1,operand_2);
		default:
			break;
		}
	}
	return create_token_from_value(result);
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

void print_stack(FILE* f, Stack* s) {
	fprintf(f, "(%d) --  ", stack_size(s));
	stack_map(s, print_token, f);
}