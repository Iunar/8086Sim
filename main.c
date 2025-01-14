#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

// ======================================== //
// 0bCCCCCCDW | 0bMM REG RMM				|
// First six bits generally give the opcode |
// D = direction of the operation, D = 1 m- |
// eans REG in second byte is the destinat- |
// ion. D = 0 means REG in second byte is   |
// the source.								|
// W = word/byte operation. W = 1 means th- |
// at the operation will be on a word, 16-b |
// its. W = 0 means that the operation will |
// be a byte, 8-bits.						|
// ======================================== //
#define OPCODE_BITS (u8)0b11111100
#define OPCODE_MOV (u8)0b10001000
#define OPCODE_LEA (u8)0b10001100

#define FLAG_D 0b00000010
#define FLAG_W 0b00000001
#define FLAG_REG 0b00111000
#define FLAG_RSM 0b00000111

#define REGISTER_BLX 0b011
#define REGISTER_CLX 0b001

// REGISTER_NL/NX (W=0/W=1)
#define REGISTER_A 0b00000000
#define REGISTER_C 0b00001000
#define REGISTER_D 0b00010000
#define REGISTER_B 0b00011000

// REGISTER AH = REGISTER AL/X + 4

#define HIBYTE(n) ((u8)((n) >> 8))
#define LOBYTE(n) ((u8)(n))

u8 instruction[3];
u8 W = 0;
u8 D = 0;

// TODO: add error checking, too lazy
u16* read_program(const char* path, u32* size_out);

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("Usage: decode <path/to/program>\n");
		return 0;
	}

	u32 file_size = 0;
	u16* file = read_program(argv[argc-1], &file_size);

	// LOBYTE   HIBYTE
	// MMMMMMDW MMREGRMM
	// 10001001 11011001
	// CHECK OPCODE, UPPER 6 BITS 0b11111100
	for(u32 i = 0; i < (file_size / 2); i++) {

		// Check opcode
		switch(LOBYTE(file[i]) & OPCODE_BITS) {
			case OPCODE_MOV: {
				instruction[0] = OPCODE_MOV;
			}break;
			case OPCODE_LEA: {
				instruction[0] = OPCODE_LEA;
			}break;
			default:
				printf("ILLEGAL INSTRUCTION\n");
				return -1;
		}

		// Check MOD (11 == reg mode)
		// CHECK W FLAG, 0b00000001
		if((LOBYTE(file[i]) & FLAG_W) == FLAG_W) {
			W = 1;
		}
		if((LOBYTE(file[i]) & FLAG_D) == FLAG_D) {
			D = 1;
		}

		// Fail if MEM != 11
		assert((HIBYTE(file[i]) & 0b11000000) == 0b11000000);

		// CHECK REG
		u8 dst_reg = (D) ? 1 : 2;
		switch((HIBYTE(file[i]) & FLAG_REG)) {
			case REGISTER_A || REGISTER_A + 4: {
				instruction[dst_reg] = REGISTER_A;
			}break;
			case REGISTER_C: {
				instruction[dst_reg] = REGISTER_C;
			}break;
			case REGISTER_D: {
				instruction[dst_reg] = REGISTER_D;
			}break;
			case REGISTER_B: {
				instruction[dst_reg] = REGISTER_B;
			}break;
			default:
				printf("ILLEGAL OR UNKNOWN REGISTER (REG)\n");
				exit(-1);
		}

		u8 src_reg = (dst_reg == 1) ? 2 : 1;
		switch((HIBYTE(file[i]) & FLAG_RSM) >> 3) {
			case (REGISTER_AL): {
				instruction[src_reg] = REGISTER_AL;
			}break;
			case (REGISTER_CL): {
				instruction[src_reg] = REGISTER_CL;
			}break;
			case (REGISTER_DL): {
				instruction[src_reg] = REGISTER_DL;
			}break;
			case (REGISTER_BL): {
				instruction[src_reg] = REGISTER_BL;
			}break;
			case (REGISTER_AH): {
				instruction[src_reg] = REGISTER_AH;
			}break;
			case (REGISTER_CH): {
				instruction[src_reg] = REGISTER_CH;
			}break;
			case (REGISTER_DH): {
				instruction[src_reg] = REGISTER_DH;
			}break;
			case (REGISTER_BH): {
				instruction[src_reg] = REGISTER_BH;
			}break;
			default:
				printf("ILLEGAL OR UNKNOWN REGISTER (R/M)\n");
				exit(-1);
		}

		// Log final instruction bad bad bad
		switch(instruction[0]) {
			case OPCODE_MOV: {
				printf("mov ");
			}break;
			case OPCODE_LEA: {
				printf("lea ");
			}break;
			default:
				printf("DEFAULT FINAL INSTRUCTION LOG SWITCH CASE (op)\n");
				exit(-1);
		}
		switch(instruction[1]) {
			case REGISTER_AL: {
				printf("a");
			}break;
			case REGISTER_CL: {
				printf("c");
			}break;
			case REGISTER_DL: {
				printf("d");
			}break;
			case REGISTER_BL: {
				printf("b");
			}break;
			default:
				printf("DEFAULT FINAL INSTRUCTION LOG SWITCH CASE (reg1)\n");
				exit(-1);
		} if(W) { printf("x, "); } else { printf("l, "); }
		switch(instruction[2]) {
			case REGISTER_AL: {
				printf("a");
			}break;
			case REGISTER_CL: {
				printf("c");
			}break;
			case REGISTER_DL: {
				printf("d");
			}break;
			case REGISTER_BL: {
				printf("b");
			}break;
			default:
				printf("DEFAULT FINAL INSTRUCTION LOG SWITCH CASE (reg2)\n");
				exit(-1);
		} if(W) { printf("x\n"); } else { printf("l\n"); }
	}

	free(file);
	return 0;
}

u16* read_program(const char* path, u32* size_out) {
	FILE* fp = fopen(path, "r");
	assert(fp);

	assert(0 == fseek(fp, 0, SEEK_END));
	
	size_t file_size = ftell(fp);
	rewind(fp);

	void* res = malloc(file_size);
	assert(res);

	assert(1 == fread(res, file_size, 1, fp));

	fclose(fp);
	
	*size_out = (u32)file_size;
	return (u16*)res;
}
