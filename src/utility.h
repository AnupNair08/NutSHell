typedef struct stackNode {
	char *comd;
	struct stackNode *next;
} stackNode;

typedef struct stack {
	stackNode *top;
	stackNode *head;
	int size;
} stack;

typedef struct doubleStack {
	stack *s1;
	stack *s2;
} doubleStack;

void printHelp();

doubleStack *init();

void push(stack *, char *);

char *pop(stack *);

doubleStack *readHistory();

char *handleArrowUp(doubleStack *);

char *handleArrowDown(doubleStack *);

void printStack(stack *);
