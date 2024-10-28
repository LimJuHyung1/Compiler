// scanner.c
#include "scanner.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* keyword[NO_KEYWORDS] = {
	"const", "else", "if", "int", "return", "void", "while"
};

enum tsymbol tnum[NO_KEYWORDS] = { tconst, telse, tif, tint, treturn, tvoid, twhile };

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

    // ���ڸ� ��� �о���̸鼭 ������ ��ȯ
    while (isdigit(ch)) {
        num = num * 10 + (ch - '0');  // '0'�� ���� ���� ���� ������ ��ȯ
        ch = fgetc(file);  // ���� ���ڸ� ���Ͽ��� �о��
    }

    ungetc(ch, file);  // ���ڰ� �ƴ� ���ڸ� ���� ���ۿ� �ٽ� ������

    return num;  // �ϼ��� ������ ��ȯ
}

struct tokenType scanner(FILE* file) {
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];

	token.number = tnull;
	token.value.id[0] = '\0'; // �ʱ�ȭ

	do {		
		while (isspace(ch = fgetc(file)));	// state 1 : skip blanks
		if (superLetter(ch)) {	// identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(file); // ���Ͽ��� ���� �б�
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, file);
			// Ű���� ���̺��� �ĺ��� ã��
			for (index = 0; index < NO_KEYWORDS; index++) {
				if (!strcmp(id, keyword[index])) {					
                    token.number = tnum[index];
                    strcpy_s(token.value.id, ID_LENGTH, keyword[index]); // keyword[index] ����
					break;
				}
			}

			if (index == NO_KEYWORDS) { // Ű���尡 �ƴ� ���
				token.number = tident;
				strcpy_s(token.value.id, ID_LENGTH, id); // �ĺ��� ����
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
                    ungetc(ch, file);    // �ǵ�����
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
                    ungetc(ch, file);    // �ǵ�����
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
                    ungetc(ch, file);    // �ǵ�����
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
                    ungetc(ch, file);    // �ǵ�����
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
                    ungetc(ch, file);    // �ǵ�����
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
                token.number = tnull; // ���� ���� ����
                break;
            }

            }
        }	// switch end
    } while (token.number == tnull);
	return token;
}	// end of scanner
