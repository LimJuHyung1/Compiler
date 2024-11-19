#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "minic.h"

void codeGen(Node* ptr) {
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
		if (p->token.number == FUNC_DEF) processFunction(p);
	// if(!mainExist) warningmsg("main does not exist");

	// step 3: generate codes for starting routine
	//		bgn		globalSize
	//		ldp
	//		call	main
	//		end
	emit1(bgn, globalSize);
	emit0(ldp);
	emitJump(call, "main");
	emit0(endop);
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
		default: print("error in SIMPLE_VAR or ARRAY_VAR\n");
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

	if (ptr->token.number != SIMPLE_VAR)printf("error in SIMPLE_VAR\n");

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
		stIndex = insert(p->token.value.id, typeSpecifier, typeQualifier, base, offset, width, 0);

		offset += size;
	}
}

void processArrayVariable(Node* ptr, int typeSpecifier, int typeQualifier){
	Node * p = ptr->son;		// variable name(=> identifier)
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

void processOperator(Node* ptr) {
	switch (ptr->token.number) {
		// assignment operator
	case ASSIGN_OP:
	{
		Node* lhs = ptr->son, * rhs = ptr->son->brother;

		// step 1: generate instructions for left-hand side if INDEX node.
		if (lhs->noderep == nonterm) {		// array variable
			lvalue = 1;
			processOperator(lhs);
			lvalue = 0;
		}

		// step 2: generate instructions for right-hand side
		if (rhs->noderep == nonterm) processOperator(rhs);
		else rv_emit(rhs);

		// step 3: generate a store instruction
		if (lhs->noderep == terminal) {		// simple variable
			stIndex = lookup(lhs->token.value.id);
			if (stIndex == -1) {
				printf("undefined variable : %s\n", lhs->token.value.id);
				return;
			}
			emit2(str, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
		else
			emit0(sti);
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
			processOperator(lhs);
			lvalue = 0;
		}

		ptr->token.number = nodeNumber;
		// step 2: code generation for repeating part
		if (lhs->noderep == nonterm) processOperator(lhs);
		else rv_emit(lhs);

		// step 3: code generation for right hand side
		if (rhs->noderep == nonterm)
			processOperator(rhs);
		else
			emit0(rhs);
		
		// step 4: emit the corresponding operation code
		switch (ptr->token.number) {
		case ADD_ASSIGN: emit0(add); break;
		case SUB_ASSIGN: emit0(sub); break;
		case MUL_ASSIGN: emit0(mult); break;
		case DIV_ASSIGN: emit0(divop); break;
		case MOD_ASSIGN: emit0(modop); break;
		}

		// steop 5: code generation for store code
		if (lhs->noderep == terminal) {
			stIndex = lookup(lhs->token.value.id);
			if (stIndex == -1) {
				printf("undefined variable : %s\n", lhs->son->token.value.id);
				return;
			}
			emit2(str, symbolTable[stIndex].base, symbolTable[stIndex].offset);
		}
		else
			emit0(sti);			
		brek;
	}
		// binary(arithmetic/relational/logical) operators
	case ADD: case SUB: case MUL: case DIV: case MOD:
	case EQ: case NE: case GT: case LT: case GE: case LE:
	case LOGICAL_AND: case LOGICAL_OR:
		// ...
	case UNARY_MINUS: case LOGICAL_NOT: // unary operators
		// ...
		// increment/decrement operators
	case PRE_INC: case PRE_DEC: case POST_INC: case POST_DEC:
		// ...
	case INDEX: 
		// ...
	case CALL:
		// ...	
	}	// end switch
}

