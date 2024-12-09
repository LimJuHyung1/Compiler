#include<stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 8   // 64비트 = 8바이트
#define BIT_SIZE 64    // 비트 순열 크기

// 64비트 입력 배열: "123456ABCD132536"
uint8_t input[8] = { 0x12, 0x34, 0x56, 0xAB, 0xCD, 0x13, 0x25, 0x36 };
// uint8_t input[8] = { 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
uint8_t output[ARRAY_SIZE] = { 0 }; // 출력 배열

int getBit(uint8_t byte, int pos);
void setBit(uint8_t* byte, int pos, int value);
void permute(uint8_t input[], uint8_t output[], int permutationTable[], int size);

int initialPermutationTable[BIT_SIZE] = {
    58, 50, 42, 34, 26, 18, 10, 2,  // 첫 줄
    60, 52, 44, 36, 28, 20, 12, 4,  // 두 번째 줄
    62, 54, 46, 38, 30, 22, 14, 6,  // 세 번째 줄
    64, 56, 48, 40, 32, 24, 16, 8,  // 네 번째 줄
    57, 49, 41, 33, 25, 17,  9, 1,  // 다섯 번째 줄
    59, 51, 43, 35, 27, 19, 11, 3,  // 여섯 번째 줄
    61, 53, 45, 37, 29, 21, 13, 5,  // 일곱 번째 줄
    63, 55, 47, 39, 31, 23, 15, 7   // 여덟 번째 줄
};
int finalPermutationTable[BIT_SIZE] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9, 49, 17, 57, 25
};

void main() {	
    permute(input, output, initialPermutationTable, BIT_SIZE);

    // 결과 출력
    printf("Permuted Array:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("0x%02X ", output[i]);
    }
    printf("\n");

    return 0;
}

// 특정 비트를 추출하는 함수 (0 또는 1 반환)
int getBit(uint8_t byte, int pos) {
    return (byte >> pos) & 1;
}

// 특정 비트를 설정하는 함수
// - byte: 비트를 설정할 대상 바이트의 포인터
// - pos: 설정할 비트의 위치 (0부터 시작, 0은 최하위 비트)
// - value: 설정할 값 (0 또는 1)
// - 기능: 지정된 위치의 비트를 1로 설정하거나 0으로 초기화합니다.
void setBit(uint8_t* byte, int pos, int value) {    
    if (value) {
        // value가 1이면 pos 위치의 비트를 1로 설정
        *byte |= (1 << pos);
    }
    else {
        // value가 0이면 pos 위치의 비트를 0으로 초기화
        *byte &= ~(1 << pos);
    }
}

// Permutation 함수 (64비트 기준)
void permute(uint8_t input[], uint8_t output[], int permutationTable[], int size) {
    uint8_t fullBits[BIT_SIZE] = { 0 };  // 입력된 비트를 풀어놓는 공간

    // Step 1: 입력 배열을 비트 단위로 추출
    int bitIndex = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int b = 7; b >= 0; b--) { // 각 바이트의 비트를 추출
            fullBits[bitIndex++] = getBit(input[i], b);
        }
    }

    // Step 2: 순열 테이블을 적용
    uint8_t permutedBits[BIT_SIZE] = { 0 };
    for (int i = 0; i < size; i++) {
        permutedBits[i] = fullBits[permutationTable[i] - 1]; // 1-based index
    }

    // Step 3: 순열된 비트를 다시 바이트 배열로 변환
    for (int i = 0; i < ARRAY_SIZE; i++) {
        output[i] = 0;  // 초기화
        for (int b = 7; b >= 0; b--) {
            setBit(&output[i], b, permutedBits[i * 8 + (7 - b)]);
        }
    }
}