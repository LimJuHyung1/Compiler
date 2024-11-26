#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "minic.h"

// #define NO_RULES 97				/* number ofo reuls */
// #define GOAL_RULE (NO_RULES + 1)	/* accept rule */
// #define NO_SYMBOLS 85			/* number of grammar symbols */
// #define NO_STATES 153			/* number of states */
#define PS_SIZE 100					/* size of parsing stack */
#define _CRT_SECURE_NO_WARNINGS

void dpush(int, int);
void semantic(int);

typedef struct nodeType {
	struct tokenType token;
	enum { terminal, nonterm } noderep;
	struct nodeType* son;
	struct nodeType* brother;
}Node;

Node* parser(FILE* file);
void error_recovery();  // error_recovery 함수 선언 추가

FILE* astFile;

enum nodeNumber {
	ACTUAL_PARAM,	ADD,			ADD_ASSIGN,		ARRAY_VAR,		ASSIGN_OP,
	CALL,			COMPOUND_ST,	CONST_NODE,		DCL,			DCL_ITEM,
	DCL_LIST,		DCL_SPEC,		DIV,			DIV_ASSIGN,		EQ,
	ERROR_NODE,		EXP_ST,			FORMAL_PARA,	FUNC_DEF,		FUNC_HEAD,
	GE,				GT,				IDENT,			IF_ELSE_ST,		IF_ST,
	INDEX,			INT_NODE,		LE,				LOGICAL_AND,	LOGICAL_NOT,
	LOGICAL_OR,		LT,				MOD,			MOD_ASSIGN,		MUL,
	MUL_ASSIGN,		NE,				NUMBER,			PARAM_DCL,		POST_DEC,
	POST_INC,		PRE_DEC,		PRE_INC,		PROGRAM,		RETURN_ST,
	SIMPLE_VAR,		STAT_LIST,		SUB,			SUB_ASSIGN,		UNARY_MINUS,
	VOID_NODE,		WHILE_ST
};

char* nodeName[] = {
	"ACTUAL_PARAM",	"ADD",			"ADD_ASSIGN",	"ARRAY_VAR",	"ASSIGN_OP",
	"CALL",			"COMPOUND_ST",	"CONST_NODE",	"DCL",			"DCL_ITEM",
	"DCL_LIST",		"DCL_SPEC",		"DIV",			"DIV_ASSIGN",	"EQ",
	"ERROR_NODE",	"EXP_ST",		"FORMAL_PARA",	"FUNC_DEF",		"FUNC_HEAD",
	"GE",			"GT",			"IDENT",		"IF_ELSE_ST",	"IF_ST",
	"INDEX",		"INT_NODE",		"LE",			"LOGICAL_AND",	"LOGICAL_NOT",
	"LOGICAL_OR",	"LT",			"MOD",			"MOD_ASSIGN",	"MUL",
	"MUL_ASSIGN",	"NE",			"NUMBER",		"PARAM_DCL",	"POST_DEC",
	"POST_INC",		"PRE_DEC",		"PRE_INC",		"PROGRAM",		"RETURN_ST",
	"SIMPLE_VAR",	"STAT_LIST",	"SUB",			"SUB_ASSIGN",	"UNARY_MINUS",
	"VOID_NODE",	"WHILE_ST"
};

int ruleName[] = {
	0,			PROGRAM,		0,			0,			0,
	0,			FUNC_DEF,		FUNC_HEAD,	DCL_SPEC,	0,
	0,			0,				0,			CONST_NODE,	INT_NODE,
	VOID_NODE,	0,				FORMAL_PARA,0,			0,
	0,			0,				PARAM_DCL,	COMPOUND_ST,DCL_LIST,
	DCL_LIST,	0,				0,			DCL,		0,
	0,			DCL_ITEM,		DCL_ITEM,	SIMPLE_VAR,	ARRAY_VAR,
	0,			0,				STAT_LIST,	0,			0,
	0,			0,				0,			0,			0,
	0,			EXP_ST,			0,			0,			IF_ST,
	IF_ELSE_ST,	WHILE_ST,		RETURN_ST,	0,			0,
	ASSIGN_OP,	ADD_ASSIGN,		SUB_ASSIGN,	MUL_ASSIGN,	DIV_ASSIGN,
	MOD_ASSIGN,	0,				LOGICAL_OR,	0,			LOGICAL_AND,
	0,			EQ,				NE,			0,			GT,
	LT,			GE,				LE,			0,			ADD,
	SUB,		0,				MUL,		DIV,		MOD,
	0,			UNARY_MINUS,	LOGICAL_NOT,PRE_INC,	PRE_DEC,
	0,			INDEX,			CALL,		POST_INC,	POST_DEC,
	0,			0,				ACTUAL_PARAM,0,			0,
	0,			0,				0
};

Node* buildNode(struct tokenType token);
Node* buildTree(int nodeNumber, int rhsLength);

void printNode(Node* pt, int indent);
void printTree(Node* pt, int indent);

int meaningfulToken(struct tokenType token);

void dumpStack();
void printToken(struct tokenType token);

int symbolStack[PS_SIZE];
Node* valueStack[PS_SIZE];
int sp, pstk[PS_SIZE];

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
		// fprintf(stderr, "Usage: %s <table_file> <source_file>\n", argv[0]);
		return EXIT_FAILURE;
	}
	// char fileName[30];
	// 두 번째 인수로 전달된 소스 파일을 열기
	FILE* file = fopen(argv[1], "r");
	
	// strcpy_s(fileName, sizeof(fileName), argv[1]);
	// fopen_s(&astFile, strcat_s(strtok(fileName, "."), ".ast"), "w");

	if (!file) {
		perror("Error opening source file");
		return EXIT_FAILURE;
	}
	// printf("start of parser\n");
	// parser(file);  // 파일 포인터를 parser 함수에 전달	
	// printf("end of parser\n");

	Node* root;
	root = parser(file);  // 파일 포인터를 parser 함수에 전달
	printTree(root, 3);

	fclose(file);  // 파일 닫기
	return 0;
}

Node* parser(FILE* file) {	
	// extern int parsingTable[NO_STATES][NO_SYMBOLS + 1];
	// extern int leftSymbol[NO_RULES + 1], rightLength[NO_RULES + 1];

	int entry, ruleNumber, lhs;
	int current_state;
	struct tokenType token;
	Node* ptr;

	sp = 0; pstk[sp] = 0;		/* inital state */
	token = scanner(file);
	while (1) {
		current_state = pstk[sp];
		entry = parsingTable[current_state][token.number];

		if (entry > 0) {		// shift action
			if (++sp > PS_SIZE) {
				printf("critical error: parsing stack overflow");
				exit(1);
			}
			dpush(token.number, entry);

			if (meaningfulToken(token)) {
				valueStack[sp] = buildNode(token);  // 의미 있는 토큰인 경우 노드 생성
			}
			else {
				valueStack[sp] = NULL;              // 의미 없는 토큰인 경우 NULL
			}

			token = scanner(file);
		}
		else if (entry < 0) {		// reduce action
			ruleNumber = -entry;
			if (ruleNumber == GOAL_RULE) { // Accept action
				if (sp - 1 < 0) {
					printf("Error: Invalid stack pointer (sp=%d)\n", sp);
					exit(1);
				}
				return valueStack[sp - 1];
			}
			// semantic(ruleNumber);			
			sp = sp - rightLength[ruleNumber] * 2;

			if (sp < 0) {
				printf("Error: Stack pointer underflow during reduction\n");
				exit(1);
			}

			ptr = buildTree(ruleName[ruleNumber], rightLength[ruleNumber]);
			lhs = leftSymbol[ruleNumber];
			current_state = parsingTable[pstk[sp]][lhs];
			dpush(lhs, current_state);
			valueStack[sp] = ptr;
		}
		else {		// error action
			printf(" === error in source ===\n");
			printf("Current Token : ");
			printToken(token);
			dumpStack();			
			error_recovery();
			token = scanner(file);
		}
	}
}

void semantic(int n) {
	printf("reduced rule number = %d\n", n);
}

void dpush(int a, int b) {
	pstk[++sp] = a;
	pstk[++sp] = b;
}

void error_recovery() {
	printf("Error recovery process\n");
}



int meaningfulToken(struct tokenType token) {
	if ((token.number == tident) || (token.number == tnumber))
		return 1;
	else return 0;
}

Node* buildNode(struct tokenType token) {
	Node* ptr;
	ptr = (Node*)malloc(sizeof(Node));
	if (!ptr) {
		printf("malloc error in buildNode()\n");
		exit(1);
	}

	ptr->token = token;
	ptr->noderep = terminal;
	ptr->son = ptr->brother = NULL;
	return ptr;
}

Node* buildTree(int nodeNumber, int rhsLength) {
	int i, j, start;
	Node* first, * ptr;

	i = sp - rhsLength + 1;
	// step 1 : find a first index with node in value stack
	while (i <= sp && valueStack[i] == NULL) i++;
	if (!nodeNumber && i > sp) return NULL;
	start = i;

	// step 2 : linking brothers
	while (i <= sp - 1) {
		j = i + 1;
		while (j <= sp && valueStack[j] == NULL) j++;
		if (j <= sp) {
			ptr = valueStack[i];
			while (ptr->brother) ptr = ptr->brother;
			ptr->brother = valueStack[j];
		}
		i = j;
	}
	first = (start > sp) ? NULL : valueStack[start];

	// step 3 : making subtree root and linking son
	if (nodeNumber) {
		ptr = (Node*)malloc(sizeof(Node));
		if (!ptr) {
			printf("malloc error in buildTree()\n");
			exit(1);
		}

		ptr->token.number = nodeNumber;
		strncpy_s(ptr->token.value.id, ID_LENGTH, "nonterm", _TRUNCATE);
		ptr->noderep = nonterm;
		ptr->son = first;
		ptr->brother = NULL;
		return ptr;
	}
	else return first;
}

void printNode(Node* pt, int indent) {
	int i;

	for (i = 1; i <= indent; i++) fprintf(astFile, " ");
	if (pt->noderep == terminal) {
		if (pt->token.number == tident) {
			fprintf(astFile, " Terminal: %s", pt->token.value.id);
		}
		else if (pt->token.number == tnumber) {
			fprintf(astFile, " Terminal: %d", pt->token.value.num);
		}
	}
	else {		// nonterminal node
		int i;
		i = (int)(pt->token.number);
		fprintf(astFile, " Nonterminal: %s", nodeName[i]);
	}
	fprintf(astFile, "\n");
}

void printTree(Node* pt, int indent) {
	Node* p = pt;
	while (p != NULL) {
		printNode(p, indent);
		if (p->noderep == nonterm) printTree(p->son, indent + 5);
		p = p->brother;
	}
}

void printToken(struct tokenType token) {
	printf("Token: %d (value: %s)\n", token.number, token.value.id);
}


void dumpStack() {
	printf("Stack state: ");
	for (int i = 0; i <= sp; i++) {
		printf("%d ", pstk[i]);
	}
	printf("\n");
}
