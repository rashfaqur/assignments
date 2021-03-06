/*
 * prob1.c
 *
 *  Created on: 
 *      Author: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* maximum length of input string (including newline character) */
#define INPUT_MAX 2048

/* enable (1) or disable (0) parentheses checking in parsing strings */
/* leave disabled for part (a); enable for part (b) */
#define PARSE_PARENS 1

/* type of token */
enum token_type {
	OPERAND, /* number */
	OPERATOR, /* operator: +, -, *, / */
#if PARSE_PARENS
	LPARENS, /* left parentheses ( */
	RPARENS /* right parentheses ) */
#endif
};

/* operator identifiers (opcodes) */
enum op {
	ADD, /* a+b */
	SUBTRACT, /* a-b (binary) */ 
	MULTIPLY, /* a*b */
	DIVIDE, /* a/b */
	NEGATE /* -a (unary) */
};

/* direction of evaluation (associativity) */
enum assoc {
	LEFT, /* left-to-right (+, binary -, *, /) */
	RIGHT /* right-to-left (unary -) */
};

/* number of operands for each operator type */
const unsigned int op_operands[] = {2, 2, 2, 2, 1};

/* order-of-operations (precedence) (0 = evaluated last) */
const unsigned int op_precedences[] = {0, 0, 1, 1, 2};

/* evaluation direction (associativity) for each precedence level */
const enum assoc op_associativity[] = {LEFT, LEFT, RIGHT};

/* contains value of token */
union token_value {
	double operand; /* numeric value for operand */
	enum op op_code; /* opcode for operators */
};

/* data structure for token */
typedef struct s_expr_token {
	union token_value value; /* numeric value or opcode */
	enum token_type type; /* type of token */

	struct s_expr_token * linked_token; /* linked token in stack/queue */
} * p_expr_token; /* p_expr_token is shorthand for "struct s_expr_token *" */

/* data structure for queue */
struct token_queue {
	p_expr_token front; /* front of queue, where tokens are dequeued */
	p_expr_token back; /* back of queue, where tokens are added */
};

/* queue functions - enqueue and dequeue */
void enqueue(struct token_queue * pqueue, const p_expr_token ptoken);
p_expr_token dequeue(struct token_queue * pqueue);

/* stack functions - push and pop */
void push(p_expr_token * ptop, const p_expr_token ptoken);
p_expr_token pop(p_expr_token * ptop);

/* creates a new token in dynamic memory (using malloc()) */
p_expr_token new_token(const enum token_type type, const union token_value value);

/* constructs a queue of tokens in infix order from a space-delimited string */
struct token_queue expr_to_infix(char * str);

/* creates a queue of tokens in postfix order from a queue of tokens in infix order */
/* postcondition: returned queue contains all the tokens, and pqueue_infix should be 
   empty */
struct token_queue infix_to_postfix(struct token_queue * pqueue_infix);

/* evalutes the postfix expression stored in the queue */
/* postcondition: returned value is final answer, and pqueue_postfix should be empty */
double evaluate_postfix(struct token_queue * pqueue_postfix);

/* handles evaluation process (calls above functions) for expression string str */
double evaluate(const char * str);

/* Function added by myself -  Show Queue */
void show_queue(struct token_queue *pqueue);

int main(void) {
	char input[INPUT_MAX];
	double ans;
	unsigned int len;

	do {
		printf("Enter an expression to evaluate: ");
		fflush(stdout);
		if (!fgets(input, INPUT_MAX, stdin))
			abort(); /* failed to read stdin */
		
		len = strlen(input);
		/* remove trailing newline character */
		if (len > 0 && input[len-1] == '\n') {
			input[len-1] = '\0';
			--len;
		}
		if (len == 0) /* empty expression signals exit */
			break;

		/* call evaluation functions */
		ans = evaluate(input);

		/* write result to stdout */
		printf("%s => %g\n", input, ans);
	} while (1);

	return 0;
}

/* enqueue (add) token to end of queue
   input: pqueue - pointer to queue
          ptoken - token pointer to add
   postcondition: token added to end of queue */
void enqueue(struct token_queue * pqueue, const p_expr_token ptoken) {
	ptoken->linked_token = NULL;
	if (pqueue->back)
		pqueue->back->linked_token = ptoken;
	else /* empty */
		pqueue->front = ptoken;
	pqueue->back = ptoken;
}

/* dequeue (remove) token from front of queue
   input: pointer to queue
   output: front token pointer (or NULL, if queue was empty)
   postcondition: token removed from queue */
p_expr_token dequeue(struct token_queue * pqueue) {
	p_expr_token ptoken = pqueue->front;
	if (pqueue->front) {
		pqueue->front = ptoken->linked_token;
		if (ptoken == pqueue->back) /* at end */
			pqueue->back = NULL;
		ptoken->linked_token = NULL;
	}
	return ptoken;
}

/* push (add) token to top of stack
   input: ptop - pointer to top token pointer of stack
          ptoken - token pointer to add
   postcondition: ptop points to the added token */
void push(p_expr_token * ptop, const p_expr_token ptoken) {
	ptoken->linked_token = *ptop;
	*ptop = ptoken;
}

/* pop (remove) token from top of stack 
   input: pointer to top token pointer of stack
   output: top token pointer (or NULL, if stack was empty)
   postcondition: ptop points to next token in stack */
p_expr_token pop(p_expr_token * ptop) {
	p_expr_token ptoken;
	if ( (ptoken = *ptop) ) {
		*ptop = ptoken->linked_token;
		ptoken->linked_token = NULL;
	}
	return ptoken;
}

/* allocate new token on heap, with specified type and value; the token is initially
   un-linked (field "linked_token" == NULL) 
   note: token must be freed using free() after use */
p_expr_token new_token(const enum token_type type, const union token_value value) {
	p_expr_token ptoken = (p_expr_token)malloc(sizeof(struct s_expr_token));
	ptoken->type = type;
	ptoken->value = value;
	ptoken->linked_token = NULL;
	return ptoken;
}

/* handles evaluation process (calls above functions) for expression string str */
/* returns the final answer */
double evaluate(const char * str) {
	char * strbuffer; /* mutable buffer for string (modified in calls to strtok()) */
	double ans; /* answer to return */
	struct token_queue queue_infix, queue_postfix;

	/* copy str into mutable buffer */
	strbuffer = strcpy((char *)malloc(strlen(str)+1),str);

	/* get queue of tokens in infix order from string buffer */
	queue_infix = expr_to_infix(strbuffer);

	show_queue(&queue_infix);

	/* get queue of tokens in postfix order from infix-ordered queue */
	queue_postfix = infix_to_postfix(&queue_infix);

//	show_queue(&queue_postfix);

	/* get answer from postfix-ordered queue */
	ans = evaluate_postfix(&queue_postfix);

	free(strbuffer); /* free memory from heap */
	return ans;
}

/* constructs a queue of tokens in infix order from a space-delimited string */
struct token_queue expr_to_infix(char * str) {
	struct token_queue queue_infix; /* queue with infix ordering */
	enum token_type type = OPERATOR;
	union token_value value;

	/* initialize the queue to empty */
	queue_infix.front = NULL;
	queue_infix.back = NULL;

	/* delimiter string for strtok() -- contains whitespace characters */
#define DELIMS_STR " \n\r\t"
	for (str = strtok(str, DELIMS_STR); str; str = strtok(NULL, DELIMS_STR)) {
		/* parse token */
		if (strlen(str) == 1) { /* operators are all 1 character */
			switch (str[0]) {
			case '+':
				type = OPERATOR;
				value.op_code = ADD;
				break;
			case '-':
				/* check previous token to distinguish between
				   negate (unary) and subtract (binary) */
				if (type == OPERATOR)
					value.op_code = NEGATE; /* unary */
#if PARSE_PARENS
				else if (type == LPARENS)
					value.op_code = NEGATE; /* unary */
#endif
				else
					value.op_code = SUBTRACT; /* binary */
				type = OPERATOR;
				break;
			case '*':
				type = OPERATOR;
				value.op_code = MULTIPLY;
				break;
			case '/':
				type = OPERATOR;
				value.op_code = DIVIDE;
				break;
#if PARSE_PARENS
			case '(':
				type = LPARENS;
				break;
			case ')':
				type = RPARENS;
				break;
#endif
			default:
				/* not an operator */
				type = OPERAND;
				value.operand = strtod(str, NULL);
			}
		} else {
			type = OPERAND;
			value.operand = strtod(str, NULL);
		}
		/* add token with parsed type and value to end of queue */
		enqueue(&queue_infix, new_token(type, value));
	}
	return queue_infix;
}

/* creates a queue of tokens in postfix order from a queue of tokens in infix order */
/* postcondition: returned queue contains all the tokens, and pqueue_infix should be 
   empty */
struct token_queue infix_to_postfix(struct token_queue * pqueue_infix) {
	/* TODO: construct postfix-ordered queue from infix-ordered queue;
	   all tokens from infix queue should be added to postfix queue or freed */

	struct token_queue queue_postfix;
	p_expr_token stack_top = NULL;
	p_expr_token temp;
	char is_left = 0;
	char is_right = 0;

	/* Dequeue infix queue untill null*/
	while((temp = dequeue(pqueue_infix))){
		is_right = 0;

		/* Operand found */
		if(temp->type == OPERAND){
			enqueue(&queue_postfix, temp);
		}
		/* Left bracket found raise flag but do nothing */
		else if(temp->type == LPARENS){
			is_left = 1;
		}
		/* Not operand */
		else{

			/* Previous operator was bracket - only push operator*/
			if(is_left){
				is_left = 0;
			}
			else{

				/* Check if top of the stack is empty */
				while(stack_top != NULL){
					if(temp->type == RPARENS){
						is_right = 1;
						enqueue(&queue_postfix, pop(&stack_top));	
						break;
					}
					/* If stack top operator is higher precedence than infix queue operator then pop */
					else if(op_precedences[temp->value.op_code] < op_precedences[stack_top->value.op_code]){
						enqueue(&queue_postfix, pop(&stack_top));
					}

					/* If precedence is same check left associativity */
					else if(op_precedences[temp->value.op_code] == op_precedences[stack_top->value.op_code]){
					
						if(op_associativity[op_precedences[stack_top->value.op_code]] == LEFT){
							enqueue(&queue_postfix, pop(&stack_top));	
						}
						else{
							break;
						}
					}

					else{
						break;
					}

				}

			}

			/* push on to stack */
			if(!is_right){
				push(&stack_top, temp);
			}
		}
	}

	while((temp = pop(&stack_top))){
		enqueue(&queue_postfix, temp);	
	}

	return queue_postfix;
}

/* evalutes the postfix expression stored in the queue */
/* postcondition: returned value is final answer, and pqueue_postfix should be empty */
double evaluate_postfix(struct token_queue * pqueue_postfix) {
	/* TODO: process postfix-ordered queue and return final answer;
	   all tokens from postfix-ordered queue is freed */
	p_expr_token p;
	p_expr_token stack_top = NULL;

	double op1, op2;
	union token_value value;

	while((p = dequeue(pqueue_postfix))){
		if(p->type == OPERAND){
			push(&stack_top, p);
		}
		else{
			switch(p->value.op_code){
				case NEGATE:
					op1 = - (pop(&stack_top)->value.operand);
					break;

				case ADD:
					op1 = pop(&stack_top)->value.operand + pop(&stack_top)->value.operand;
					break;

				case MULTIPLY:
					op1 = pop(&stack_top)->value.operand * pop(&stack_top)->value.operand;
					break;

				case DIVIDE:
					op2 = pop(&stack_top)->value.operand;
					op1 = pop(&stack_top)->value.operand;
					op1 = op1 / op2;
					break;

				case SUBTRACT:
					op2 = pop(&stack_top)->value.operand;
					op1 = pop(&stack_top)->value.operand;
					op1 = op1 - op2;
					break;
			}

			value.operand = op1;
			push(&stack_top, new_token(OPERAND, value));
		}
	}

	return pop(&stack_top)->value.operand;	
}

void show_queue(struct token_queue *pqueue){
	p_expr_token temp = pqueue->front;
	
	printf("Queue: ");

	for(; temp != NULL; temp = temp->linked_token){
		if(temp->type == OPERAND){
			printf("%lf ", temp->value.operand);
		}
		else if(temp->type == LPARENS){
			printf("%c ", '(');
		}
		else if(temp->type == RPARENS){
			printf("%c ", ')');
		}
		else{
			switch(temp->value.op_code){
				case ADD:
					printf("%c ", '+');
					break;

				case SUBTRACT:
				case NEGATE:
					printf("%c ", '-');
					break;

				case MULTIPLY:
					printf("%c ", '*');
					break;

				case DIVIDE:
					printf("%c ", '/');
					break;
			}
		}
	}

	printf("\n");
}
