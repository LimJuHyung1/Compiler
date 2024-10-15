#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>  // For _kbhit() and _getch()

#define _CRT_SECURE_NO_WARNINGS

#define NO_KEYWORDS 7
#define ID_LENGTH 12

char* keyword[NO_KEYWORDS] = {
	"const", "else", "if", "int", "return", "void", "while"
};

void lexicalError(int n);
int superLetter(char ch);
int superLetterOrDigit(char ch);
int getIntNum(FILE* file, char ch);

enum tsymbol { tnull = -1,
tnot, tnotequ, tmod, tmodAssign, tident, tnumber, tand, tlparen, trparen, tmul, tmulAssign, 
tplus, tinc, taddAssign, tcomma, tminus, tdec, tsubAssign, tdiv, tdivAssgin, tsemicolon, tless, tlesse, 
tassign, tequal, tgreat, tgreate, tlbracket, trbracket, teof,
tconst, telse, tif, tint, treturn, tvoid, twhile, tlbrace, tor, trbrace};

enum tsymbol tnum[NO_KEYWORDS] = {
	tconst, telse, tif, tint, treturn , tvoid, twhile
};

struct tokenType {
	int number;		// token number
	union {
		char id[ID_LENGTH];
		int num;
	} value;		// token value
};

enum tnumber { TEOF, TIDEN, TNUM, TASSIGN, TADD, TSEMI, TDOT, TBEGIN, TEND, TERROR };

struct tokenType scanner(FILE* file) {
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];

	token.number = tnull;
	token.value.id[0] = '\0'; // 초기화

	do {		
		while (isspace(ch = fgetc(file)));	// state 1 : skip blanks
		if (superLetter(ch)) {	// identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(file); // 파일에서 문자 읽기
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, file);
			// 키워드 테이블에서 식별자 찾기
			for (index = 0; index < NO_KEYWORDS; index++) {
				if (!strcmp(id, keyword[index])) {					
                    token.number = tnum[index];
                    strcpy_s(token.value.id, ID_LENGTH, keyword[index]); // keyword[index] 복사
					break;
				}
			}

			if (index == NO_KEYWORDS) { // 키워드가 아닌 경우
				token.number = tident;
				strcpy_s(token.value.id, ID_LENGTH, id); // 식별자 복사
			}
		}	// end of identifier or keyword
		else if (isdigit(ch)) {	// integer constant
            int num = 0;
            do {
                num = 10 * num + (int)(ch - '0');
                ch = fgetc(file);
            } while (isdigit(ch));
            ungetc(ch, file); // retract
            token.number = tnumber;
            token.value.num = num;
		}
        else
        {
            switch (ch) {	// special character
            case '/':   // state 10
                ch = fgetc(file);
                if (ch == '*')
                    do {
                        while (ch != '*') ch = fgetc(file);
                        ch = fgetc(file);
                    } while (ch != '/');
                else if (ch == '/')
                    while (fgetc(file) != '\n');
                else if (ch == '=') token.number = tdivAssgin;
                else {
                    token.number = tdiv;
                    ungetc(ch, stdin);
                }                
                break;
            case '!':	// state 17
                ch = fgetc(file);
                if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "!=");
                    token.number = tnotequ;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), "!");
                    token.number = tnot;
                    ungetc(ch, file);    // 되돌리기
                }
                break;
            case '%':	// state 20
                ch = fgetc(file);
                if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "%=");
                    token.number = tmodAssign;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), "%");
                    token.number = tmod;
                    ungetc(ch, file);    // 되돌리기
                }
                break;
            case '&':	// state 23
                ch = fgetc(file);
                if (ch == '&') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "&&");
                    token.number = tand;
                }
                else {
                    lexicalError(2);
                    ungetc(ch, file);    // 되돌리기
                }
                break;
            case '*':	// state 25
                ch = getchar();
                if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "*=");
                    token.number = tmulAssign;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), "*");
                    token.number = tmul;
                    ungetc(ch, file);    // 되돌리기
                }
                break;
            case '+':	// state 28
                ch = fgetc(file);
                if (ch == '+') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "++");
                    token.number = tinc;
                }
                else if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "+=");
                    token.number = taddAssign;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), "+");
                    token.number = tplus;
                    ungetc(ch, file);    // 되돌리기
                }
                break;
            case '-':	// state 32
                ch = fgetc(file);
                if (ch == '-') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "--");
                    token.number = tdec;
                }
                else if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "-=");
                    token.number = tsubAssign;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), "-");
                    token.number = tminus;
                    ungetc(ch, file);
                }
                break;
            case '<':	// state 36
                ch = fgetc(file);
                if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "<=");
                    token.number = tlesse;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), "<");
                    token.number = tless;
                    ungetc(ch, file);
                }
                break;
            case '=':	// state 39
                ch = fgetc(file);
                if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "==");
                    token.number = tequal;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), "=");
                    token.number = tassign;
                    ungetc(ch, file);
                }
                break;
            case '>':	// state 42
                ch = fgetc(file);
                if (ch == '=') {
                    strcpy_s(token.value.id, sizeof(token.value.id), ">=");
                    token.number = tgreate;
                }
                else {
                    strcpy_s(token.value.id, sizeof(token.value.id), ">");
                    token.number = tgreat;
                    ungetc(ch, file);
                }
                break;
            case '|':	// state 45
                ch = fgetc(file);
                if (ch == '|') {
                    strcpy_s(token.value.id, sizeof(token.value.id), "||");
                    token.number = tor;
                }
                else {
                    lexicalError(3);
                    ungetc(ch, file);
                }
                break;

            case '(': strcpy_s(token.value.id, sizeof(token.value.id), "("); token.number = tlparen;			break;
            case ')': strcpy_s(token.value.id, sizeof(token.value.id), ")"); token.number = trparen;			break;
            case ',': strcpy_s(token.value.id, sizeof(token.value.id), ","); token.number = tcomma;			    break;
            case ';': strcpy_s(token.value.id, sizeof(token.value.id), ";"); token.number = tsemicolon;		    break;
            case '[': strcpy_s(token.value.id, sizeof(token.value.id), "["); token.number = tlbracket;		    break;
            case ']': strcpy_s(token.value.id, sizeof(token.value.id), "]"); token.number = trbracket;		    break;
            case '{': strcpy_s(token.value.id, sizeof(token.value.id), "{"); token.number = tlbrace;			break;
            case '}': strcpy_s(token.value.id, sizeof(token.value.id), "}"); token.number = trbrace;			break;
            case EOF:
                token.number = teof;
                break;

            default: {
                printf("Current character: %c\n", ch);
                lexicalError(4);
                token.number = tnull; // 오류 상태 설정
                break;
            }

            }
        }	// switch end
    } while (token.number == tnull);
	return token;
}	// end of scanner

// 메인 함수
int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: %s <input_file>\n", argv[0]);
		return 1;
	}

	FILE* file = fopen(argv[1], "r");

	if (file == NULL) {
		printf("Error: Could not open file %s\n", argv[1]);
		return 1;
	}
	else {
		printf("File %s opened successfully.\n", argv[1]); // 파일이 열렸는지 확인
	}

	struct tokenType token;

	// 파일로부터 토큰을 읽고 출력하는 루프
	while (1) {
		token = scanner(file);  // scanner 함수 호출하여 토큰 읽기

		if (token.number == teof) {
			printf("End of file reached.\n");
			break;
		}				

		if (token.number == tident) {
			// printf("식별자 입니다 - ");
			printf("Token itself: %-10s | Token number: %-10d | Token value: ",
				token.value.id, token.number);
			printf("%s\n", token.value.id);
		}
		else if (token.number == tnumber) {
			// printf("상수 입니다 - ");
			printf("Token itself: %-10d | Token number: %-10d | Token value: ",
                token.value.num, token.number);
			printf("%d\n", token.value.num);            
		}
		else {
			// printf("그외 입니다 - ");
			printf("Token itself: %-10s | Token number: %-10d | Token value: ",
				token.value.id, token.number);
			printf("0\n");
		}
	}

	fclose(file);
	return 0;
}

void lexicalError(int n) {
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next charcter must be &.\n");
		break;
	case 3: printf("next charcter must be |.\n");
		break;
	case 4: printf("invalid character!!!\n");
		break;
	}
}

int superLetter(char ch) {
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch) {
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

int getIntNum(FILE* file, char ch) {
	int num = 0;

	// 숫자를 계속 읽어들이면서 정수로 변환
	while (isdigit(ch)) {
		num = num * 10 + (ch - '0');  // '0'을 빼서 실제 숫자 값으로 변환
		ch = fgetc(file);  // 다음 문자를 파일에서 읽어옴
	}

	ungetc(ch, file);  // 숫자가 아닌 문자를 파일 버퍼에 다시 돌려줌

	return num;  // 완성된 정수를 반환
}
