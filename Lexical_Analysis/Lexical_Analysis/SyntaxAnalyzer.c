#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
// #include "minic.tbl"
#define NO_RULES 70				/* number ofo reuls */
#define GOAL_RULE (NO_RULES + 1)	/* accept rule */
#define NO_SYMBOLS 74			/* number of grammar symbols */
#define NO_STATES 131			/* number of states */
#define PS_SIZE 100					/* size of parsing stack */

void dpush(int, int);
int sp, pstk[PS_SIZE];
void semantic(int);
void parser(FILE* file);
void error_recovery();  // error_recovery 함수 선언 추가
void loadDataFromFile(const char* filename);

int parsingTable[NO_STATES][NO_SYMBOLS + 1];
int leftSymbol[NO_RULES + 1], rightLength[NO_RULES + 1];

int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <table_file> <source_file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// 첫 번째 인수로 전달된 .tbl 파일에서 데이터를 로드
	loadDataFromFile(argv[1]);

	// 두 번째 인수로 전달된 소스 파일을 열기
	FILE* file = fopen(argv[2], "r");
	if (!file) {
		perror("Error opening source file");
		return EXIT_FAILURE;
	}
	printf("start of parser\n");
	parser(file);  // 파일 포인터를 parser 함수에 전달
	printf("end of parser\n");

	fclose(file);  // 파일 닫기
	return 0;
}

void parser(FILE* file) {	
	int entry, ruleNumber, lhs;
	int current_state;
	struct tokenType token;

	sp = 0; pstk[sp] = 0;		/* inital state */
	token = scanner(file);
	while (1) {
		current_state = pstk[sp];
		entry = parsingTable[current_state][token.number];
		if (entry > 0) {
			dpush(token.number, entry);
			token = scanner(file);
		}
		else if (entry < 0) {
			ruleNumber = -entry;
			if (ruleNumber == GOAL_RULE) {
				printf(" *** valid source ***\n");
				return;
			}
			semantic(ruleNumber);
			sp = sp - rightLength[ruleNumber] * 2;
			lhs = leftSymbol[ruleNumber];
			current_state = parsingTable[pstk[sp]][lhs];
			dpush(lhs, current_state);
		}
		else {
			printf(" === error in source ===\n");
			error_recovery();
		}
	}
}

void semantic(int n) {
	printf("reduced rule number = %d\n", n);
}

void dpush(int a, int b) {
	pstk[++sp] = a;
	pstk[++sp] = b;
}

void error_recovery() {
	// error recovery를 위한 로직 작성
	printf("Error recovery process\n");
}

// .tbl 파일에서 데이터를 로드하는 함수
void loadDataFromFile(const char* filename) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		perror("Error opening table file");
		exit(EXIT_FAILURE);
	}

	// leftSymbol 배열 읽기
	for (int i = 0; i <= NO_RULES; i++) {
		if (fscanf_s(file, "%d,", &leftSymbol[i]) != 1) {
			fprintf(stderr, "Error reading leftSymbol at index %d\n", i);
			exit(EXIT_FAILURE);
		}
	}

	// rightLength 배열 읽기
	for (int i = 0; i <= NO_RULES; i++) {
		if (fscanf_s(file, "%d,", &rightLength[i]) != 1) {
			fprintf(stderr, "Error reading rightLength at index %d\n", i);
			exit(EXIT_FAILURE);
		}
	}

	// parsingTable 배열 읽기
	for (int i = 0; i < NO_STATES; i++) {
		for (int j = 0; j <= NO_SYMBOLS; j++) {
			if (fscanf_s(file, "%d,", &parsingTable[i][j]) != 1) {
				fprintf(stderr, "Error reading parsingTable at state %d, symbol %d\n", i, j);
				exit(EXIT_FAILURE);
			}
		}
	}

	fclose(file);
	printf("Data loaded successfully from %s\n", filename);
}