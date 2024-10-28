// lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define NO_KEYWORDS 7
#define ID_LENGTH 12

// 키워드 배열
extern char* keyword[NO_KEYWORDS];

// 오류 처리 함수
void lexicalError(int n);

// 문자 관련 함수
int superLetter(char ch);
int superLetterOrDigit(char ch);
int getIntNum(FILE* file, char ch);

// 토큰 및 심볼 열거형
enum tsymbol {
    tnull = -1, tnot, tnotequ, tmod, tmodAssign, tident, tnumber, tand, tlparen, trparen,
    tmul, tmulAssign, tplus, tinc, taddAssign, tcomma, tminus, tdec, tsubAssign, tdiv,
    tdivAssgin, tsemicolon, tless, tlesse, tassign, tequal, tgreat, tgreate, tlbracket,
    trbracket, teof, tconst, telse, tif, tint, treturn, tvoid, twhile, tlbrace, tor, trbrace
};

// 키워드에 해당하는 tsymbol 값을 저장하는 배열
extern enum tsymbol tnum[NO_KEYWORDS];

// 토큰 구조체 정의
struct tokenType {
    int number;  // token number
    union {
        char id[ID_LENGTH];
        int num;
    } value;  // token value
};

// 토큰을 스캔하는 함수
struct tokenType scanner(FILE* file);

#endif // LEXER_H
