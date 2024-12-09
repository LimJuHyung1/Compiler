#include<stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 8   // 64��Ʈ = 8����Ʈ
#define BIT_SIZE 64    // ��Ʈ ���� ũ��

// 64��Ʈ �Է� �迭: "123456ABCD132536"
uint8_t input[8] = { 0x12, 0x34, 0x56, 0xAB, 0xCD, 0x13, 0x25, 0x36 };
// uint8_t input[8] = { 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
uint8_t output[ARRAY_SIZE] = { 0 }; // ��� �迭

int getBit(uint8_t byte, int pos);
void setBit(uint8_t* byte, int pos, int value);
void permute(uint8_t input[], uint8_t output[], int permutationTable[], int size);

int initialPermutationTable[BIT_SIZE] = {
    58, 50, 42, 34, 26, 18, 10, 2,  // ù ��
    60, 52, 44, 36, 28, 20, 12, 4,  // �� ��° ��
    62, 54, 46, 38, 30, 22, 14, 6,  // �� ��° ��
    64, 56, 48, 40, 32, 24, 16, 8,  // �� ��° ��
    57, 49, 41, 33, 25, 17,  9, 1,  // �ټ� ��° ��
    59, 51, 43, 35, 27, 19, 11, 3,  // ���� ��° ��
    61, 53, 45, 37, 29, 21, 13, 5,  // �ϰ� ��° ��
    63, 55, 47, 39, 31, 23, 15, 7   // ���� ��° ��
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

    // ��� ���
    printf("Permuted Array:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("0x%02X ", output[i]);
    }
    printf("\n");

    return 0;
}

// Ư�� ��Ʈ�� �����ϴ� �Լ� (0 �Ǵ� 1 ��ȯ)
int getBit(uint8_t byte, int pos) {
    return (byte >> pos) & 1;
}

// Ư�� ��Ʈ�� �����ϴ� �Լ�
// - byte: ��Ʈ�� ������ ��� ����Ʈ�� ������
// - pos: ������ ��Ʈ�� ��ġ (0���� ����, 0�� ������ ��Ʈ)
// - value: ������ �� (0 �Ǵ� 1)
// - ���: ������ ��ġ�� ��Ʈ�� 1�� �����ϰų� 0���� �ʱ�ȭ�մϴ�.
void setBit(uint8_t* byte, int pos, int value) {    
    if (value) {
        // value�� 1�̸� pos ��ġ�� ��Ʈ�� 1�� ����
        *byte |= (1 << pos);
    }
    else {
        // value�� 0�̸� pos ��ġ�� ��Ʈ�� 0���� �ʱ�ȭ
        *byte &= ~(1 << pos);
    }
}

// Permutation �Լ� (64��Ʈ ����)
void permute(uint8_t input[], uint8_t output[], int permutationTable[], int size) {
    uint8_t fullBits[BIT_SIZE] = { 0 };  // �Էµ� ��Ʈ�� Ǯ����� ����

    // Step 1: �Է� �迭�� ��Ʈ ������ ����
    int bitIndex = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int b = 7; b >= 0; b--) { // �� ����Ʈ�� ��Ʈ�� ����
            fullBits[bitIndex++] = getBit(input[i], b);
        }
    }

    // Step 2: ���� ���̺��� ����
    uint8_t permutedBits[BIT_SIZE] = { 0 };
    for (int i = 0; i < size; i++) {
        permutedBits[i] = fullBits[permutationTable[i] - 1]; // 1-based index
    }

    // Step 3: ������ ��Ʈ�� �ٽ� ����Ʈ �迭�� ��ȯ
    for (int i = 0; i < ARRAY_SIZE; i++) {
        output[i] = 0;  // �ʱ�ȭ
        for (int b = 7; b >= 0; b--) {
            setBit(&output[i], b, permutedBits[i * 8 + (7 - b)]);
        }
    }
}