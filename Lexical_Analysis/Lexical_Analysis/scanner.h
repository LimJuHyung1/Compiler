// lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define NO_KEYWORDS 7
#define ID_LENGTH 12

// Ű���� �迭
extern char* keyword[NO_KEYWORDS];

// ���� ó�� �Լ�
void lexicalError(int n);

// ���� ���� �Լ�
int superLetter(char ch);
int superLetterOrDigit(char ch);
int getIntNum(FILE* file, char ch);

// ��ū �� �ɺ� ������
enum tsymbol {
    tnull = -1, tnot, tnotequ, tmod, tmodAssign, tident, tnumber, tand, tlparen, trparen,
    tmul, tmulAssign, tplus, tinc, taddAssign, tcomma, tminus, tdec, tsubAssign, tdiv,
    tdivAssgin, tsemicolon, tless, tlesse, tassign, tequal, tgreat, tgreate, tlbracket,
    trbracket, teof, tconst, telse, tif, tint, treturn, tvoid, twhile, tlbrace, tor, trbrace
};

// Ű���忡 �ش��ϴ� tsymbol ���� �����ϴ� �迭
extern enum tsymbol tnum[NO_KEYWORDS];

// ��ū ����ü ����
struct tokenType {
    int number;  // token number
    union {
        char id[ID_LENGTH];
        int num;
    } value;  // token value
};

// ��ū�� ��ĵ�ϴ� �Լ�
struct tokenType scanner(FILE* file);

#endif // LEXER_H
