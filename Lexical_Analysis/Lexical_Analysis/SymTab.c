// SymTab.c
#include "SymTab.h"

// Global Variables
char* typeName[] = {
    "none", "void", "int"
};

char* qualifierName[] = {
    "NONE", "FUNC_TYPE", "PARAM_TYPE", "CONST_TYPE", "VAR_TYPE"
};

struct SymbolEntry symbolTable[SYMTAB_SIZE];
int symbolTableTop;
int hashBucket[HASH_BUCKET_SIZE];
int levelStack[SYMTAB_SIZE];
int levelTop;
int base, offset;
int width;

/*------------------ TABLE INITIALIZE FUNCTION -----------------------------*/
void initSymbolTable() {
    int i;

    symbolTableTop = -1;
    levelTop = -1;
    base = 1; offset = 1;

    for (i = 0; i < HASH_BUCKET_SIZE; i++)
        hashBucket[i] = -1;

    for (i = 0; i < LEVEL_STACK_SIZE; i++)
        levelStack[i] = 0;
}

/*--------------------- HASH FUNCTION --------------------------------------*/
int hash(char* symbolName) {
    int hashValue = 0;

    while (*symbolName)
        hashValue += *symbolName++;

    return (hashValue % HASH_BUCKET_SIZE);
}

/*------------------ LOOKUP FUNCTION ---------------------------------------*/
int lookup(char* symbol) {
    int stIndex = hash(symbol);
    for (stIndex = hashBucket[stIndex];
        stIndex >= 0 && strcmp(symbol, symbolTable[stIndex].symbolName);
        stIndex = symbolTable[stIndex].nextIndex);

    if (stIndex < 0)
        printf("undefined identifier: %s\n", symbol);

    return stIndex;
}

/*-------------------- INSERT SYMBOL FUNCTION ------------------------------*/
int insert(char* symbol, int specifier, int qualifier, int base, int offset,
    int width, int initialValue) {
    int hashValue = hash(symbol);
    int i;

    for (i = hashBucket[hashValue];
        i >= 0 && strcmp(symbol, symbolTable[i].symbolName);
        i = symbolTable[i].nextIndex);

    if (i >= 0 && base == symbolTable[i].base) {
        printf("multiply defined identifier : %s\n", symbol);
        return -1;
    }

    if (symbolTableTop >= SYMTAB_SIZE - 1) {
        printf("symbol table overflow\n");
        dumpSymbolTable();
        exit(1);
    }

    symbolTableTop++;
    strcpy_s(symbolTable[symbolTableTop].symbolName, sizeof(symbolTable[symbolTableTop].symbolName), symbol);
    symbolTable[symbolTableTop].typeSpecifier = specifier;
    symbolTable[symbolTableTop].typeQualifier = qualifier;
    symbolTable[symbolTableTop].base = base;
    symbolTable[symbolTableTop].offset = offset;
    symbolTable[symbolTableTop].width = width;
    symbolTable[symbolTableTop].initialValue = initialValue;

    symbolTable[symbolTableTop].nextIndex = hashBucket[hashValue];
    hashBucket[hashValue] = symbolTableTop;

    return symbolTableTop;
}

/*------------------ SET BLOCK FUNCTION ------------------------------------*/
void set() {
    ++levelTop;
    if (levelTop >= LEVEL_STACK_SIZE)
        printf("error: level stack overflow.\n");

    levelStack[levelTop] = symbolTableTop;
    ++base; offset = 1;
}

/*--------------------- RESET BLOCK FUNCTION -------------------------------*/
void reset(void) {
    int hashValue, i;

    i = symbolTableTop;
    symbolTableTop = levelStack[levelTop];
    levelTop--;

    for (; i > symbolTableTop; i--) {
        hashValue = hash(symbolTable[i].symbolName);
        hashBucket[hashValue] = symbolTable[i].nextIndex;
    }
    base--;
}

/*-------------------   Dump Symbol Table   -----------------------------*/
void dumpSymbolTable() {
    int i;

    printf(" === Dump Symbol Table ===\n");
    for (i = 0; i <= symbolTableTop; i++)
        printf("id = %-12s : \ttype = %s, \tqualifier = %s, \tbase = %d, \toffset = %d \twidth = %d, \tvalue = %d\n",
            symbolTable[i].symbolName,
            typeName[symbolTable[i].typeSpecifier],
            qualifierName[symbolTable[i].typeQualifier],
            symbolTable[i].base,
            symbolTable[i].offset,
            symbolTable[i].width,
            symbolTable[i].initialValue);
}

/*------------------- generate Symbol Table -----------------------------*/
void genSym(int base) {
    int stIndex;

    for (stIndex = 0; stIndex <= symbolTableTop; stIndex++) {
        if (symbolTable[stIndex].typeQualifier == FUNC_TYPE ||
            symbolTable[stIndex].typeQualifier == CONST_TYPE)
            continue;

        if (base == symbolTable[stIndex].base)
            printf("Emit symbol: base=%d, offset=%d, width=%d\n",
                symbolTable[stIndex].base,
                symbolTable[stIndex].offset,
                symbolTable[stIndex].width);
    }
}
