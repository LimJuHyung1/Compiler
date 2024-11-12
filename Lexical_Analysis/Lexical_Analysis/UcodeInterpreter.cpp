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
	/*"mod"*/	100, /*"and"*/	10, /*"or"*/		10, /*"gt"*/		20, /*"lt"*/		20, /*"ge"*/		20, /*"le"*/		20, /*"eq"*/		20, /*"ne"*/		20,
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
	if (s2 != NULL) {  // s2가 NULL이 아닌 경우에만 출력
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



