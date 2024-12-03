#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define SYM_LENGTH 31
#define SYMTAB_SIZE 200
#define HASH_BUCKET_SIZE 97
#define LEVEL_STACK_SIZE 10

// Type Specifier Enum
typedef enum {
    NON_SPECIFIER, VOID_TYPE, INT_TYPE
} TypeSpecifier;

// Type Qualifier Enum
typedef enum {
    NON_QUALIFIER, FUNC_TYPE, PARAM_TYPE, CONST_TYPE, VAR_TYPE
} TypeQualifier;

// Dimension Enum
typedef enum {
    ZERO_DIMENSION, ONE_DIMENSION
} Dimension;

// Structure Definition for Symbol Entry
struct SymbolEntry {
    char symbolName[SYM_LENGTH];
    int typeSpecifier;
    int typeQualifier;
    int base;
    int offset;
    int width;          // Size
    int initialValue;   // Initial value
    int nextIndex;      // Link to next entry
};

// Global Variables
extern char* typeName[];
extern char* qualifierName[];
extern struct SymbolEntry symbolTable[SYMTAB_SIZE];
extern int symbolTableTop;
extern int hashBucket[HASH_BUCKET_SIZE];
extern int levelStack[SYMTAB_SIZE];
extern int levelTop;
extern int base;
extern int offset;
extern int width;

// Function Declarations
void initSymbolTable();
int hash(char* symbolName);
int lookup(char* symbol);
int insert(char* symbol, int specifier, int qualifier, int base, int offset, int width, int initialValue);
void set();
void reset();
void dumpSymbolTable();
void genSym(int base);

#endif // SYMTAB_H
