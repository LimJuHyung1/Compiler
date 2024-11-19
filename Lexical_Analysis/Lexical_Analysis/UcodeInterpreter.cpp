#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

const int MAXINSTR = 2000;
const int MAXLABELS = 300;
const int STACKSIZE = 20000;
const int LABELSIZE = 10;
const int NO_OPCODES = 41;

std::ifstream inputFile;
std::ofstream outputFile;

void errmsg(char* s, char* s2);

enum opcode {
	notop, neg, incop, decop, dup, swp, add, sub, mult, divop,
	modop, andop, orop, gt, lt, ge, le, eq, ne,
	lod, ldc, lda, ldi, ldp, str, sti, ujp, tjp, fjp,
	call, ret, retv, chkh, chkl, nop, proc, endop, bgn, sym,
	dump, none
};

std::string opcodeName[NO_OPCODES] = {
	"notop", "neg", "inc", "dec", "dup", "swp", "add", "sub", "mult", "div",
	"mod", "and", "or", "gt", "lt", "ge", "le", "eq", "ne",
	"lod", "ldc", "lda", "ldi", "ldp", "str", "sti", "ujp", "tjp", "fjp",
	"call", "ret", "retv", "chkh", "chkl", "nop", "proc", "end", "bgn", "sym",
	"dump", "none"
};

int executable[NO_OPCODES] = {
	/*"notop"*/	1, /*"neg"*/	1, /*"inc"*/	1, /*"dec"*/	1, /*"dup"*/	1, /*"swp"*/	1, /*"add"*/	1, /*"sub"*/	1, /*"mult"*/	1, /*"div"*/	1,
	/*"mod"*/	1, /*"and"*/	1, /*"or"*/		1, /*"gt"*/		1, /*"lt"*/		1, /*"ge"*/		1, /*"le"*/		1, /*"eq"*/		1, /*"ne"*/		1,
	/*"lod"*/	1, /*"ldc"*/	1, /*"lda"*/	1, /*"ldi"*/	1, /*"ldp"*/	1, /*"str"*/	1, /*"sti"*/	1, /*"ujp"*/	1, /*"tjp"*/	1, /*"fjp"*/	1,
	/*"call"*/	1, /*"ret"*/	1, /*"retv"*/	1, /*"chkh"*/	1, /*"chkl"*/	1, /*"nop"*/	0, /*"proc"*/	1, /*"end"*/	0, /*"bgn"*/	0, /*"sym"*/	0,
	/*"dump"*/	1, /*"none"*/	0
};

int opcodeCycle[NO_OPCODES] = {
	/*"notop"*/	5, /*"neg"*/	5, /*"inc"*/	1, /*"dec"*/	1, /*"dup"*/	5, /*"swp"*/	10, /*"add"*/	10, /*"sub"*/	10, /*"mult"*/	50, /*"div"*/	100,
	/*"mod"*/	100, /*"and"*/	10, /*"or"*/	10, /*"gt"*/	20, /*"lt"*/	20, /*"ge"*/	20, /*"le"*/	20, /*"eq"*/	20, /*"ne"*/	20,
	/*"lod"*/	5, /*"ldc"*/	5, /*"lda"*/	5, /*"ldi"*/	10, /*"ldp"*/	10, /*"str"*/	5, /*"sti"*/	10, /*"ujp"*/	10, /*"tjp"*/	10, /*"fjp"*/	10,
	/*"call"*/	30, /*"ret"*/	30, /*"retv"*/	30, /*"chkh"*/	5, /*"chkl"*/	5, /*"nop"*/	0, /*"proc"*/	30, /*"end"*/	0, /*"bgn"*/	0, /*"sym"*/	0,
	/*"dump"*/	100, /*"none"*/	0
};

int staticCnt[NO_OPCODES], dynamicCnt[NO_OPCODES];
enum { FALSE, TRUE };;
enum procIndex {READPROC = -1, WRITEPROC = -2, LFPROC = -3, UNDEFINED = -1000};

typedef struct {
	int opcdoe;
	int value1;
	int value2;
	int value3;
} Instruction;

Instruction instrBuf[MAXINSTR];

void errmsg(char* s, char* s2 = NULL) {
	std::cerr << "error !!! " << s;
	if (s2 != NULL) {  // s2�� NULL�� �ƴ� ��쿡�� ���
		std::cerr << ": " << s2;
	}
	std::cerr << "\n";
	exit(1);
}

class  UcodeiStack {
	int size; 
	int sp;
	int* stackArray;

public:
	void push(int);
	int pop();
	int top() { return sp; }
	void spSet(int n) { sp = n; }
	void dump();
	int& operator[](int);
	UcodeiStack(int);
	~UcodeiStack() { delete[] stackArray; }
};

class Label {
	struct fixUpList {
		int instrAddress;
		struct fixUpList* next;
	};

	struct labelEntry {
		char labelName[LABELSIZE];
		int address;
		struct fixUpList* instrList;
	};

	struct labelEntry labelTable[MAXLABELS];
	int labelCnt;
	void addFix(struct fixUpList*, int);

public:
	void insertLabel(char[], int);
	void findLabel(char[], int);
	void checkUndefinedLabel();
	Label();
	virtual ~Label();
};

class Assemble {
	int instrCnt;
	char lineBuffer[80];
	int bufIndex;
	Label labelProcess;
	char label[LABELSIZE];
	void getLabel();
	int getOpcode();
	int getOperand();
	void instrWrite();

public:
	void assemble();
	int startAddr;
	Assemble() {
		instrCnt = 0;
	}
};

class Interpret {
	UcodeiStack stack;
	int arBase;
	long int tcycle;
	long int exeCount;
	void predefinedProc(int);
	int findAddr(int);
	void statistic();
public:
	void execute(int);
	Interpret();
	virtual ~Interpret() {}
};

UcodeiStack::UcodeiStack(int size) {
	stackArray = new int[size];
	sp = -1;
	push(-1); push(-1); push(-1); push(0);
	push(0); push(0); push(-1); push(1);
}

void UcodeiStack::push(int value) {
	if (sp == STACKSIZE) errmsg("push()", "Stack Overflow...");
	stackArray[++sp] = value;
}

int UcodeiStack::pop() {
	if (sp == 0) errmsg("pop()", "Stack Underflow");
	return stackArray[sp--];
}

void UcodeiStack::dump() {
	int i;

	std::cout << "stack dump : (address : value)\n";
	for (i = sp - 10; i <= sp; ++i) {
		std::cout << ' ' << i << " : " << stackArray[i] << "\n";	
	}
	std::cout << '\n';
}

int& UcodeiStack::operator[](int index) {
	return stackArray[index];
}

Label::Label() {
	int index;

	labelCnt = 2;
	strcpy(labelTable[0].labelName, "read");
	labelTable[0].address = READPROC;
	labelTable[0].instrList = NULL;

	strcpy(labelTable[1].labelName, "write");
	labelTable[1].address = WRITEPROC;
	labelTable[1].instrList = NULL;

	strcpy(labelTable[2].labelName, "lf");
	labelTable[2].address = LFPROC;
	labelTable[2].instrList = NULL;

	for (index = 3; index < MAXLABELS; index++) {
		labelTable[index].address = UNDEFINED;
	}
}

void Label::insertLabel(char label[], int value) {
	struct fixUpList* ptr;
	int index;

	for (index = 0; (index <= labelCnt) && strcmp(labelTable[index].labelName, label); index++);
	labelTable[index].address = value;
	if (index > labelCnt) {
		strcpy(labelTable[index].labelName, label);
		labelCnt = index;
		labelTable[index].instrList = NULL;
	}
	else {
		ptr = labelTable[index].instrList;
		labelTable[index].instrList = NULL;
		while (ptr) {	// backpatching
			instrBuf[ptr->instrAddress].value1 = value;
			ptr = ptr->next;
		}
	}
}

void Label::findLabel(char label[], int instr) {
	struct fixUpList* ptr;
	int index;

	for (index = 0; (index <= labelCnt) && strcmp(labelTable[index].labelName, label); index++);

	if (index > labelCnt) {		// not found
		strcpy(labelTable[index].labelName, label);
		labelCnt = index;
		ptr = new fixUpdateList;

		if (ptr == NULL) errmsg("findLabel()", "Out of memory -- new");
		labelTable[index].instrList = ptr;
		ptr->instrAddress = instr;
		ptr->next = NULL;
	}
	else {						// found
		ptr = labelTable[index].instrList;
		if (ptr) addFix(ptr, instr);
		else instrBuf[instr].value1 = labelTable[index].address;
	}
}

void Label::addFix(struct) {
	struct fixUpList* succ;

	while(prev->next) prev = prev->next;
	succ = new fixUpList;
	if (succ == NULL)errmsg("addFix()", "Out of memory");
	succ->instrAddress = instr;
	succ->next = NULL;
	prev->next = succ;
	//delete succ;
}

void Label::checkUndefinedLabel() {
	int index;

	for (index = 0; index <= labelCnt; index++) {
		if (labelTable[index].address == UNDEFINED) {
			errmsg("undefined label", labelTable[index].labelName);
		}
	}
}

void Assemble::getLabel() {
	int i;
}

