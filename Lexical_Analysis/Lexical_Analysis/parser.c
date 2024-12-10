#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "minic.h"
#include "SymTab.h"
#include "EmitCode.h"

// #define NO_RULES 97				/* number ofo reuls */
// #define GOAL_RULE (NO_RULES + 1)	/* accept rule */
// #define NO_SYMBOLS 85			/* number of grammar symbols */
// #define NO_STATES 153			/* number of states */
#define PS_SIZE 100					/* size of parsing stack */
#define _CRT_SECURE_NO_WARNINGS

typedef struct nodeType {
	struct tokenType token;
	enum { terminal, nonterm } noderep;
	struct nodeType* son;
	struct nodeType* brother;
}Node;

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

Node* parser(FILE* file);
void error_recovery(FILE* file);  // error_recovery 함수 선언 추가

Node* buildNode(struct tokenType token);
Node* buildTree(int nodeNumber, int rhsLength);
void printNode(Node* pt, int indent);
void printTree(Node* pt, int indent);

int meaningfulToken(struct tokenType token);
void dpush(int, int);
void semantic(int);
void dumpStack();
void printToken(struct tokenType token);

int symbolStack[PS_SIZE];
Node* valueStack[PS_SIZE];
int sp, pstk[PS_SIZE];

FILE* astFile;  // 외부 변수 선언
FILE* ucodeFile;

void codeGen(FILE* file, Node* ptr);
void processDeclaration(Node* ptr);
void processSimpleVariable(Node* ptr, int typeSpecifier, int typeQualifier);
void processArrayVariable(Node* ptr, int typeSpecifier, int typeQualifier);
void rv_emit(FILE* file, Node* ptr);
void processCondition(FILE* file, Node* ptr);
void processFuncHeader(Node* ptr);
void processFunction(FILE* file, Node* ptr);

int returnWithValue;
int initalValue;
int lvalue;

int main(int argc, char* argv[]) {
	char fileName[30];
	char astFileName[260];     // AST 파일 이름 저장
	char ucodeFileName[260];   // UCode 파일 이름 저장
	Node* root;

	FILE* file = fopen(argv[1], "r");

	// argv[1] 체크
	if (argc < 2 || argv[1] == NULL) {
		fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// argv[1] 복사
	errno_t err = strcpy_s(fileName, sizeof(fileName), argv[1]);
	if (err != 0) {
		fprintf(stderr, "Error copying file name.\n");
		return EXIT_FAILURE;
	}

	// AST 파일 이름 생성
	char* baseName = strtok(fileName, ".");
	if (baseName == NULL) {
		fprintf(stderr, "Invalid file name format.\n");
		return EXIT_FAILURE;
	}

	// .ast 확장자 추가
	err = strcpy_s(astFileName, sizeof(astFileName), baseName);
	if (err != 0) {
		fprintf(stderr, "Error creating AST file name.\n");
		return EXIT_FAILURE;
	}
	strcat_s(astFileName, sizeof(astFileName), ".ast");

	// .uco 확장자 추가
	err = strcpy_s(ucodeFileName, sizeof(ucodeFileName), baseName);
	if (err != 0) {
		fprintf(stderr, "Error creating UCode file name.\n");
		return EXIT_FAILURE;
	}
	strcat_s(ucodeFileName, sizeof(ucodeFileName), ".uco");

	// AST 파일 열기
	err = fopen_s(&astFile, astFileName, "w");
	if (err != 0 || astFile == NULL) {
		fprintf(stderr, "Error opening AST file: %s\n", astFileName);
		return EXIT_FAILURE;
	}

	// UCode 파일 열기
	err = fopen_s(&ucodeFile, ucodeFileName, "w");
	if (err != 0 || ucodeFile == NULL) {
		fprintf(stderr, "Error opening UCode file: %s\n", ucodeFileName);
		fclose(astFile);
		return EXIT_FAILURE;
	}

	// errno_t err = fopen_s(&astFile, "output.ast", "w");
	// errno_t err2 = fopen_s(&ucodeFile, "output.uco", "w");

	printf(" *** start of Mini C Compiler\n");
	if (argc != 2) {
		icg_error(1);
		exit(1);
	}
	// strcpy(fileName, argv[1]);
	strcpy_s(fileName, sizeof(fileName), argv[1]);
	printf("   * source file name: %s\n", fileName);

	/*
	if ((sourceFile = fopen(fileName, 'r')) == NULL) {
		icg_error(2);
		exit(1);
	}*/


	printf(" === start of Parser\n");
	root = parser(file);
	printTree(root, 0);
	codeGen(file, root);
	printf(" *** end of Mini C Compiler\n");
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
			sp++;
			if (sp > PS_SIZE) {
				printf("critical error: parsing stack overflow");
				exit(1);
			}
			symbolStack[sp] = token.number;
			pstk[sp] = entry;			
			valueStack[sp] = meaningfulToken(token) ? buildNode(token) : NULL;
			
			/*
			if (meaningfulToken(token)) {
				valueStack[sp] = buildNode(token);  // 의미 있는 토큰인 경우 노드 생성
			}
			else {
				valueStack[sp] = NULL;              // 의미 없는 토큰인 경우 NULL
			}
			*/

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

			/*
			if (sp < 0) {
				printf("Error: Stack pointer underflow during reduction\n");
				exit(1);
			}
			*/

			ptr = buildTree(ruleName[ruleNumber], rightLength[ruleNumber]);
			sp = sp - rightLength[ruleNumber];
			lhs = leftSymbol[ruleNumber];
			current_state = parsingTable[pstk[sp]][lhs];
			sp++;
			symbolStack[sp] = lhs;
			pstk[sp] = current_state;
			valueStack[sp] = ptr;
		}
		else {		// error action
			printf(" === error in source ===\n");
			printf("Current Token : ");
			printToken(token);
			dumpStack();			
			error_recovery(file);
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

void error_recovery(FILE* file) {
	struct tokenType tok;
	int parenthesisCount, braceCount;
	int i;

	// step 1: skip to the semicolon
	parenthesisCount = braceCount = 0;
	while (true) {
		tok = scanner(file);
		if (tok.number == teof) exit(1);
		if (tok.number == tlparen) parenthesisCount++;
		else if (tok.number == trparen) parenthesisCount--;

		if (tok.number == tlbrace) braceCount++;
		else if (tok.number == trbrace) braceCount--;

		if ((tok.number == tsemicolon) && (parenthesisCount <= 0) && (braceCount <= 0))
			break;
	}

	// step 2: adjust state stack
	for (i = sp; i >= 0; i--) {
		// statement_list -> statement_list . statement
		if (pstk[i] == 36) break;	// second statement part

		// statement_list -> . statement
		// statement_list -> . statement_list statement
		if (pstk[i] == 24) break;	// first statement part

		// declaration_list -> declaration_list . declaration
		if (pstk[i] == 25) break;	// second internal dcl

		// declaration_list -> declaration
		// declaration_list -> declaration_list . declaration
		if (pstk[i] == 17) break;	// second statement part

		// external declaration
		// external dcl -> . declaration
		if (pstk[i] == 2) break;	// after first external dcl
		if (pstk[i] == 0) break;	// first external declaration
	}
	sp = i;
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
		ptr->token.value.num = NULL;
		// strncpy_s(ptr->token.value.id, ID_LENGTH, "nonterm", _TRUNCATE);
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
	if (token.number == tident)
		printf("%s", token.value.id);
	else if (token.number == tnumber)
		printf("%d", token.value.num);	
	else
		printf("%s", nodeName[token.number]);
}

void dumpStack() {
	int i, start;

	if (sp > 10) {
		start = sp - 10;
	}
	else start = 0;

	printf("\n *** dump state stack : ");
	for (i = start; i <= sp; ++i)
		printf("%d ", pstk[i]);

	printf("\n *** dump symbol stack : ");
	for (i = start; i <= sp; ++i)
		printf("%d ", symbolStack[i]);
	printf("\n");
}









void codeGen(FILE* file, Node* ptr) {
	Node* p;
	int globalSize;

	initSymbolTable();

	// step 1 : process the declaration part
	for (p = ptr->son; p; p = p->brother) {
		if (p->token.number == DCL) processDeclaration(p->son);
		else if (p->token.number == FUNC_DEF) processFuncHeader(p->son);
		else icg_error(3);
	}

	// dumpSymbolTable();
	globalSize = offset - 1;
	// printf("size of global variables = %d\n", globalSize);

	genSym(base);

	// step 2: process the function part
	for (p = ptr->son; p; p = p->brother)
		if (p->token.number == FUNC_DEF) processFunction(file, p);
	// if(!mainExist) warningmsg("main does not exist");

	// step 3: generate codes for starting routine
	//		bgn		globalSize
	//		ldp
	//		call	main
	//		end
	emit1(file, bgn, globalSize);
	emit0(file, ldp);
	emitJump(file, call, "main");
	emit0(file, endop);
}

void processDeclaration(Node* ptr) {
	int typeSpecifier, typeQualifier;
	Node* p, * q;

	if (ptr->token.number != DCL_SPEC) icg_error(4);

	// printf("processDeclaration\n");
	// step 1: process DCL_SPEC
	typeSpecifier = INT_TYPE;			// default type
	typeQualifier = VAR_TYPE;
	p = ptr->son;
	while (p) {
		if (p->token.number == INT_NODE) typeSpecifier = INT_TYPE;
		else if (p->token.number == CONST_NODE)
			typeQualifier = CONST_TYPE;
		else {		// AUTO, EXTERN, REGISTER, FLOAT, DOUBLE, SIGNED, UNSIGNED
			printf("not yet implemented\n");
			return;
		}
		p = p->brother;
	}

	// step 2: process DCL_ITEM
	p = ptr->brother;
	if (p->token.number != DCL_ITEM) icg_error(5);

	while (p) {
		q = p->son;		// SYMPLE_VAR or ARRAY_VAR
		switch (q->token.number) {
		case SIMPLE_VAR:		// simple variable
			processSimpleVariable(q, typeSpecifier, typeQualifier);
			break;
		case ARRAY_VAR:			// array variable
			processArrayVariable(q, typeSpecifier, typeQualifier);
			break;
		default: printf("error in SIMPLE_VAR or ARRAY_VAR\n");
			break;
		}	// end switch
		p = p->brother;
	}	// end while
}


void processSimpleVariable(Node* ptr, int typeSpecifier, int typeQualifier) {
	Node* p = ptr->son;		// variable name(=> identifier)
	Node* q = ptr->brother;	// inital value part
	int stIndex, size, initial;
	int sign = 1;

	if (ptr->token.number != SIMPLE_VAR) printf("error in SIMPLE_VAR\n");

	if (typeQualifier == CONST_TYPE) {		// constant type
		if (q == NULL) {
			printf("%s must have a constant value\n", ptr->son->token.value.id);
			return;
		}

		if (q->token.number == UNARY_MINUS) {
			sign = -1;
			q = q->son;
		}

		initalValue = sign * q->token.value.num;

		stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, 0/*base*/, 0/*offset*/, 0/*width*/, initalValue);
	}
	else {			// variable type
		size = typeSize(typeSpecifier);
		// stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, base, offset, width, 0);
		stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, base, offset, width, 0);

		offset += size;
	}
}

void processArrayVariable(Node* ptr, int typeSpecifier, int typeQualifier) {
	Node* p = ptr->son;		// variable name(=> identifier)
	int stIndex, size;

	if (ptr->token.number != ARRAY_VAR) {
		printf("error in ARRAY_VAR\n");
		return;
	}
	if (p->brother == NULL)	// no size
		printf("array size must be specified\n");
	else size = p->brother->token.value.num;

	size *= typeSize(typeSpecifier);

	stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, base, offset, size, 0);
	offset += size;
}

void processOperator(FILE* file, Node* ptr) {
	int stIndex;

	switch (ptr->token.number) {
		// assignment operator
	case ASSIGN_OP:
	{
		Node* lhs = ptr->son, *rhs = ptr->son->brother;

		// step 1: generate instructions for left-hand side if INDEX node.
		if (lhs->noderep == nonterm) {		// array variable			
			lvalue = 1;
			processOperator(file, lhs);
			lvalue = 0;
		}

		// step 2: generate instructions for right-hand side
		if (rhs->noderep == nonterm) processOperator(file, rhs);
		else rv_emit(file, rhs);

		// step 3: generate a store instruction
		if (lhs->noderep == terminal) {		// simple variable
			stIndex = lookup(lhs->token.value.id);
			if (stIndex == -1) {
				printf("undefined variable : %s\n", lhs->token.value.id);
				return;
			}
			emit2(file, str, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
		else
			emit0(file, sti);
		break;
	}

	// complex assignment operators
	case ADD_ASSIGN: case SUB_ASSIGN:
	case MUL_ASSIGN: case DIV_ASSIGN:
	case MOD_ASSIGN:
	{
		Node* lhs = ptr->son, * rhs = ptr->son->brother;
		int nodeNumber = ptr->token.number;

		ptr->token.number = ASSIGN_OP;

		//step 1: code generation for left hand side
		if (lhs->noderep == nonterm) {
			lvalue = 1;
			processOperator(file, lhs);
			lvalue = 0;
		}

		ptr->token.number = nodeNumber;
		// step 2: code generation for repeating part
		if (lhs->noderep == nonterm) processOperator(file, lhs);
		else rv_emit(file, lhs);

		// step 3: code generation for right hand side
		if (rhs->noderep == nonterm)
			processOperator(file, rhs);
		else
			emit0(file, rhs->token.value.num);

		// step 4: emit the corresponding operation code
		switch (ptr->token.number) {
		case ADD_ASSIGN: emit0(file, add); break;
		case SUB_ASSIGN: emit0(file, sub); break;
		case MUL_ASSIGN: emit0(file, mult); break;
		case DIV_ASSIGN: emit0(file, divop); break;
		case MOD_ASSIGN: emit0(file, modop); break;
		}

		// steop 5: code generation for store code
		if (lhs->noderep == terminal) {
			stIndex = lookup(lhs->token.value.id);
			if (stIndex == -1) {
				printf("undefined variable : %s\n", lhs->son->token.value.id);
				return;
			}
			emit2(file, str, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
		else
			emit0(file, sti);
		break;
	}
	// binary(arithmetic/relational/logical) operators
	case ADD: case SUB: case MUL: case DIV: case MOD:
	case EQ: case NE: case GT: case LT: case GE: case LE:
	case LOGICAL_AND: case LOGICAL_OR:
	{
		Node* lhs = ptr->son, * rhs = ptr->son->brother;

		// step 1: visit left operand
		if (lhs->noderep == nonterm) processOperator(file, lhs);
		else rv_emit(file, lhs);

		// step 2: visit right operand
		if (rhs->noderep == nonterm) processOperator(file, rhs);
		else rv_emit(file, rhs);

		// step 3: visit root
		switch (ptr->token.number) {
			case ADD:	emit0(file, add);	break;		// arithmetic operators
			case SUB:	emit0(file, sub);	break;
			case MUL:	emit0(file, mult);	break;
			case DIV:	emit0(file, divop);	break;
			case MOD:	emit0(file, modop);	break;
			case EQ:	emit0(file, eq);	break;		// relational operators
			case NE:	emit0(file, ne);	break;
			case GT:	emit0(file, gt);	break;
			case LT:	emit0(file, lt);	break;
			case GE:	emit0(file, ge);	break;
			case LE:	emit0(file, le);	break;
			case LOGICAL_AND:	emit0(file, andop);	break;	// logical operators
			case LOGICAL_OR:	emit0(file, orop);	break;			
		}
		break;
	}

	case UNARY_MINUS: case LOGICAL_NOT: // unary operators
	{
		Node* p = ptr->son;

		if (p->noderep == nonterm) processOperator(file, p);
		else rv_emit(file, p);

		switch (ptr->token.number)
		{
			case UNARY_MINUS: emit0(file, neg);		break;
			case LOGICAL_NOT: emit0(file, notop);	break;
		}
		break;
	}		


	case PRE_INC: case PRE_DEC: case POST_INC: case POST_DEC:
	{
		Node* p = ptr->son; Node* q;
		int amount = 1;
		if (p->noderep == nonterm) processOperator(file, p);	// compute operand
		else rv_emit(file, p);

		q = p;
		while (q->noderep != terminal) q = q->son;

		if (!q || (q->token.number != tident)) {
			printf("increment/decrement operators can not be applied in expression\n");
			return;
		}

		stIndex = lookup(q->token.value.id);
		if (stIndex == -1) return;

		switch (ptr->token.number) {
		case PRE_INC: emit0(file, incop);
			// if (isOperation(ptr)) emit0(file, dup);
			break;
		case PRE_DEC: emit0(file, decop);
			// if (isOperation(ptr)) emit0(file, dup);
			break;
		case POST_INC: emit0(file, incop);
			// if (isOperation(ptr)) emit0(file, dup);
			break;
		case POST_DEC: emit0(file, decop);
			// if (isOperation(ptr)) emit0(file, dup);
			break;
		}

		if (p->noderep == terminal) {
			stIndex = lookup(p->token.value.id);
			if (stIndex == -1) return;
			emit2(file, str, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
		else if (p->token.number == INDEX) {	// compute index
			lvalue = 1;
			processOperator(file, p);
			lvalue = 0;
			emit0(file, swp);
			emit0(file, sti);
		}
		else printf("error in increment/decrement operators\n");
		break;
	}
	case INDEX:
	{
		Node* indexExp = ptr->son->brother;

		if (indexExp->noderep == nonterm) processOperator(file, indexExp);
		else rv_emit(file, indexExp);
		stIndex = lookup(ptr->son->token.value.id);
		if (stIndex == -1) {
			printf("undefined variable : %s\n", ptr->son->token.value.id);
			return;
		}

		emit2(file, lda, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		emit0(file, add);

		if (!lvalue) emit0(file, ldi); break;	// rvalue
	}
	case CALL:
	{
		Node* p = ptr->son;		// function name
		char* functionName;
		int noArguments;
		/*
		if (checkPredefined(p))		// predefined(Library) functions
			break;
			*/

		// handle for user function
		functionName = p->token.value.id;
		stIndex = lookup(functionName);
		if (stIndex == -1)	break;	// undefined function !!!
		noArguments = symbolTable[stIndex].width;

		emit0(file, ldp);

		p = p->brother;		// ACTUAL_PARAM
		while (p) {			// processing actual arguments
			if (p->noderep == nonterm) processOperator(file, p);
			else rv_emit(file, p);

			noArguments--;
			p = p->brother;
		}

		if (noArguments > 0)
			printf("%s: too few actual arguments", functionName);
		if (noArguments < 0)
			printf("%s: too many actual arguments", functionName);
		emitJump(file, call, ptr->son->token.value.id);
		break;
	}
	}	// end switch
}

void rv_emit(FILE* file, Node* ptr) {
	int stIndex;

	if (ptr->token.number == tnumber)
		emit1(file, ldc, ptr->token.value.num);
	else {
		stIndex = lookup(ptr->token.value.id);
		if (stIndex == -1) return;
		if (symbolTable[stIndex].typeQualifier == CONST_TYPE)	// constant
			emit1(file, ldc, symbolTable[stIndex].initialValue);
		else if (symbolTable[stIndex].width > 1)		// array var
			emit2(file, lda, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		else			// simple var
		{
			emit2(file, lod, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
	}
}


void processStatement(FILE* file, Node* ptr) {
	Node* p;

	switch (ptr->token.number) {
	case COMPOUND_ST:
	{
		p = ptr->son->brother;		// STAT_LIST
		p = p->son;

		while (p) {
			processStatement(file, p);
			p = p->brother;
		}
		break;
	}
	case EXP_ST: {
		if (ptr->son != NULL) processOperator(file, ptr->son);
		break;
	}
		
	case RETURN_ST: {
		if (ptr->son != NULL) {
			returnWithValue = 1;
			p = ptr->son;
			if (p->noderep == nonterm)
				processOperator(file, p);	// return value
			else rv_emit(file, p);
			emit0(file, retv);
		}
		else
			emit0(file, ret);
		break;
	}		
	case IF_ST:
	{
		char label[50];

		genLabel(label);
		processCondition(file, ptr->son);		// condition part
		emitJump(file, fjp, label);
		processStatement(file, ptr->son->brother);		// true part
		emitLabel(file, label);
	}
	break;
	
	case IF_ELSE_ST:
	{
		// int const LABEL_SIZE = 50;
		char label1[50], label2[50];

		genLabel(label1);
		genLabel(label2);
		processCondition(file, ptr->son);		// condition part
		emitJump(file, fjp, label1);
		processStatement(file, ptr->son->brother);		// true part
		emitJump(file, ujp, label2);
		emitLabel(file, label1);
		processStatement(file, ptr->son->brother->brother);		// false part
		emitLabel(file, label2);
	}
	break;
	
	case WHILE_ST:
	{
		char label1[50], label2[50];

		genLabel(label1);
		genLabel(label2);
		emitLabel(file, label1);
		processCondition(file, ptr->son);		// condition part
		emitJump(file, fjp, label2);
		processStatement(file, ptr->son->brother);		// loop body
		emitJump(file, ujp, label1);
		emitLabel(file, label2);
	}
	break;
	
	default:
		printf("not yet implemented\n");
		break;
	}	// end switch
}

void processCondition(FILE* file, Node* ptr) {
	if (ptr->noderep == nonterm) processOperator(file, ptr);
	else rv_emit(file, ptr);
}

void processFuncHeader(Node* ptr) {
	int noArguments, returnType;
	int stIndex;
	Node* p;

	// printf("processFuncHeader\n");
	if (ptr->token.number != FUNC_HEAD)
		printf("error in processFuncHeader\n");

	// stpe 1: process the function return type
	p = ptr->son->son;
	while (p) {
		if (p->token.number == INT_NODE) returnType = INT_TYPE;
		else if (p->token.number == VOID_NODE) returnType = VOID_TYPE;
		else printf("invalid function return type\n");
		p = p->brother;
	}

	// step 2: count the number of formal parameters
	p = ptr->son->brother->brother;	// FORMAL_PARA
	p = p->son;	// PARAM_DCL

	noArguments = 0;
	while (p) {
		noArguments++;
		p = p->brother;
	}

	// step 3: insert the function name
	stIndex = insert(ptr->son->brother->token.value.id, returnType, FUNC_TYPE, 1/*base*/, 0/*offset*/, noArguments/*width*/, 0/*initialValue*/);
	// if(!strcmp("main", functionName)) mainExist = 1;
}

int typeSize(int typeSpecifier) {
	if (typeSpecifier == INT_TYPE) return 1;
	else {
		printf("not yet implemented\n");
		exit(0);
	}
}

void processFunction(FILE* file, Node* ptr) {
	Node* p, * q, * r;
	base++;

	if (ptr->token.number != FUNC_DEF) printf(" error in processFunction\n");

	// step 1: process formal parameters
	p = ptr->son->son->brother->brother; // FORMAL_PARA
	p = p->son;			// PARAM_DCL

	while (p) {
		processFuncHeader(p->son);
		p = p->brother;
	}

	// step 2: process the declaration part in function body
	q = ptr->son->brother;	// COMPOUND_ST
	q = q->son->son;	// DCL
	while (q) {
		processDeclaration(q->son);
		q = q->brother;
	}
	dumpSymbolTable();

	//step 3: emit the function start code(proc p1 p2 p3)
	printf("%-10s proc %5d %5d %5d\n", ptr->son->son->brother->token.value.id, offset - 1, base, 2);
	fprintf(ucodeFile, "%-10s proc  %5d %5d %5d\n", ptr->son->son->brother->token.value.id, offset - 1, base, 2);
	genSym(base);

	// step 4: process the statement part in function body
	r = ptr->son->brother->son->brother; // COMPOUND_ST-STAT_LIST
	r = ptr->son;	// STATEMENT

	while (r) {
		processStatement(file, r);
		r = r->brother;
	}

	// step 5: check if return type and return value
	// step 6: generate the ending codes
	if (returnWithValue == 0) emit0(file, ret);

	emit0(file, endop);

	offset = 1;
	returnWithValue = 0;
}

void emitprocessFunction(FILE* file, Node* ptr) {
	// 함수의 실제 구현 내용 작성
	fprintf(file, "Processing function: %s\n", ptr->token.value.id);
}