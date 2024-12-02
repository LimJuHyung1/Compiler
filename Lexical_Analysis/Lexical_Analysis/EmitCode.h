#ifndef EMITCODE_H
#define EMITCODE_H

#include <stdio.h>

// Define the number of opcodes
#define NUMBER_OF_OPCODES 40

// Define the opcode enum
typedef enum {
    notop, neg, incop, decop, dup, swp, add, sub,
    mult, divop, modop, andop, orop, gt, lt, ge,
    le, eq, ne, lod, ldc, lda, ldi, ldp,
    str, sti, ujp, tjp, fjp, call, ret, retv,
    chkh, chkl, nop, proc, endop, bgn, sym, none
} opcode;

// Declare the mnemonic array
extern char* mnemonic[NUMBER_OF_OPCODES];

// Declare the functions
void emit0(FILE* ucodeFile, opcode op);
void emit1(FILE* ucodeFile, opcode op, int num);
void emit2(FILE* ucodeFile, opcode op, int base, int offset);
void emit3(FILE* ucodeFile, opcode op, int p1, int p2, int p3);
void emitLabel(FILE* ucodeFile, char* label);
void emitJump(FILE* ucodeFile, opcode op, char* label);
void emitSym(FILE* ucodeFile, int base, int offset, int size);
void emitFunc(FILE* ucodeFile, char* label, int base, int offset, int size);

#endif // EMITCODE_H
