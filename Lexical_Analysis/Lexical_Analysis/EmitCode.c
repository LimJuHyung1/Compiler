#include "EmitCode.h"
#include <string.h>

char* mnemonic[NUMBER_OF_OPCODES] = {
    "notop", "neg", "inc", "dec", "dup", "swp", "add", "sub",
    "mult", "div", "mod", "and", "or", "gt", "lt", "ge",
    "le", "eq", "ne", "lod", "ldc", "lda", "ldi", "ldp",
    "str", "sti", "ujp", "tjp", "fjp", "call", "ret", "retv",
    "chkh", "chkl", "nop", "proc", "end", "bgn", "sym", "none"
};

void emit0(FILE* ucodeFile, opcode op) {
    fprintf(ucodeFile, "           ");
    fprintf(ucodeFile, "%-10s\n", mnemonic[op]);
}

void emit1(FILE* ucodeFile, opcode op, int num) {
    fprintf(ucodeFile, "           ");
    fprintf(ucodeFile, "%-10s %5d\n", mnemonic[op], num);
}

void emit2(FILE* ucodeFile, opcode op, int base, int offset) {
    fprintf(ucodeFile, "           ");
    fprintf(ucodeFile, "%-10s %5d %5d\n", mnemonic[op], base, offset);
}

void emit3(FILE* ucodeFile, opcode op, int p1, int p2, int p3) {
    fprintf(ucodeFile, "           ");
    fprintf(ucodeFile, "%-10s %5d %5d %5d\n", mnemonic[op], p1, p2, p3);
}

void emitLabel(FILE* ucodeFile, char* label) {
    int i, noBlanks;
    fprintf(ucodeFile, "%s", label);
    noBlanks = 12 - strlen(label);
    for (i = 1; i < noBlanks; ++i)
        fprintf(ucodeFile, " ");
    fprintf(ucodeFile, "%-10s\n", mnemonic[nop]);
}

void emitJump(FILE* ucodeFile, opcode op, char* label) {
    fprintf(ucodeFile, "           ");
    fprintf(ucodeFile, "%-10s %s\n", mnemonic[op], label);
}

void emitSym(FILE* ucodeFile, int base, int offset, int size) {
    fprintf(ucodeFile, "           ");
    fprintf(ucodeFile, "%-10s %5d %5d %5d\n", mnemonic[sym], base, offset, size);
}

void emitFunc(FILE* ucodeFile, char* label, int base, int offset, int size) {
    int i, noBlanks;
    fprintf(ucodeFile, "%s", label);
    noBlanks = 12 - strlen(label);
    for (i = 1; i < noBlanks; ++i)
        fprintf(ucodeFile, " ");
    fprintf(ucodeFile, "%-10s", mnemonic[proc]);
    fprintf(ucodeFile, " ");
    fprintf(ucodeFile, "%5d %5d %5d\n", base, offset, size);
}
