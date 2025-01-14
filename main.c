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

#define FLAG_D (u8)0b00000010
#define FLAG_W (u8)0b00000001
#define REG_BITS 0b00011000
#define RSM_BITS 0b00000011

#define REGISTER_BLX 0b011
#define REGISTER_CLX 0b001

// REGISTER_NL/NX (W=0/W=1)
#define REG_HIGH_BIT 0b00100000 // The bit set when the register is of high varient
#define REGISTER_A   0b00000000
#define REGISTER_C   0b00001000
#define REGISTER_D   0b00010000
#define REGISTER_B   0b00011000

#define HIBYTE(n) ((u8)((n) >> 8))
#define LOBYTE(n) ((u8)(n))

char instruction_str[32];
u8 instruction[3];
u8 REG_RSM_HIBITS[2]; // { REG, RSM }: default;
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

		// Check if high bit is set for REG
		REG_RSM_HIBITS[0] = ( (HIBYTE(file[i]) & REG_HIGH_BIT) == REG_HIGH_BIT) ? 1 : 0;

		// Check who contains dest/src (index)
		//u8 dst_reg = (D) ? 1 : 2;
		//u8 src_reg = (dst_reg == 1) ? 2 : 1;

		// Check contents of REG
		switch((HIBYTE(file[i]) & REG_BITS)) {
			case REGISTER_A: {
				instruction[1] = REGISTER_A;
			}break;
			case REGISTER_C: {
				instruction[1] = REGISTER_C;
			}break;
			case REGISTER_D: {
				instruction[1] = REGISTER_D;
			}break;
			case REGISTER_B: {
				instruction[1] = REGISTER_B;
			}break;
			default:
				printf("ILLEGAL OR UNKNOWN OPERAND (REG)\n");
				exit(-1);
		}

		// Check if high bit is set
		REG_RSM_HIBITS[1] = ((HIBYTE(file[i]) & (REG_HIGH_BIT >> 3)) == (REG_HIGH_BIT >> 3)) ? 1 : 0;
		// Check contents of R/M
		switch((HIBYTE(file[i]) & RSM_BITS) << 3) {
			case (REGISTER_A): {
				instruction[2] = REGISTER_A;
			}break;
			case (REGISTER_C): {
				instruction[2] = REGISTER_C;
			}break;
			case (REGISTER_D): {
				instruction[2] = REGISTER_D;
			}break;
			case (REGISTER_B): {
				instruction[2] = REGISTER_B;
			}break;
			default:
				printf("ILLEGAL OR UNKNOWN OPERAND (R/M)\n");
				exit(-1);
		}

		if(!D) {
			// swap REG and RSM
			u8 tmp = instruction[2];
			instruction[2] = instruction[1];
			instruction[1] = tmp;

			// swap hibits
			tmp = REG_RSM_HIBITS[1];
			REG_RSM_HIBITS[1] = REG_RSM_HIBITS[0];
			REG_RSM_HIBITS[0] = tmp;
		}

		switch(instruction[0]) {
			case OPCODE_MOV: {
				printf("mov ");
			}break;
			default: {
				printf("ILL ");
			}break;
		}

		// dst
		switch(instruction[1]) {
			case REGISTER_A: {
				if(W && !REG_RSM_HIBITS[0]) { // AX
					printf("ax, ");
				} else if(!W && !REG_RSM_HIBITS[0]) { // AL
					printf("al, ");
				} else if(W && REG_RSM_HIBITS[0]) { // SP
					printf("sp, ");
				} else if(!W && REG_RSM_HIBITS[0]) { // AH
					printf("ah, ");
				}
			}break;
			case REGISTER_C: {
				if(W && !REG_RSM_HIBITS[0]) { // CX
					printf("cx, ");
				} else if(!W && !REG_RSM_HIBITS[0]) { // CL
					printf("cl, ");
				} else if(W && REG_RSM_HIBITS[0]) { // BP
					printf("bp, ");
				} else if(!W && REG_RSM_HIBITS[0]) { // CH
					printf("ch, ");
				}
			}break;
			case REGISTER_D: {
				if(W && !REG_RSM_HIBITS[0]) { // DX
					printf("dx, ");
				} else if(!W && !REG_RSM_HIBITS[0]) { // DL
					printf("dl, ");
				} else if(W && REG_RSM_HIBITS[0]) { // SI
					printf("si, ");
				} else if(!W && REG_RSM_HIBITS[0]) { // DH
					printf("dh, ");
				}
			}break;
			case REGISTER_B: {
				if(W && !REG_RSM_HIBITS[0]) { // BX
					printf("bx, ");
				} else if(!W && !REG_RSM_HIBITS[0]) { // BL
					printf("bl, ");
				} else if(W && REG_RSM_HIBITS[0]) { // DI
					printf("di, ");
				} else if(!W && REG_RSM_HIBITS[0]) { // BH
					printf("bh, ");
				}
			}break;
			default: {
				printf("ILL, ");
			}break;
		}

		// src
		switch(instruction[2]) {
			case REGISTER_A: {
				if(W && !REG_RSM_HIBITS[1]) { // AX
					printf("ax");
				} else if(!W && !REG_RSM_HIBITS[1]) { // AL
					printf("al");
				} else if(W && REG_RSM_HIBITS[1]) { // SP
					printf("sp");
				} else if(!W && REG_RSM_HIBITS[1]) { // AH
					printf("ah");
				}
			}break;
			case REGISTER_C: {
				if(W && !REG_RSM_HIBITS[1]) { // CX
					printf("cx");
				} else if(!W && !REG_RSM_HIBITS[1]) { // CL
					printf("cl");
				} else if(W && REG_RSM_HIBITS[1]) { // BP
					printf("bp");
				} else if(!W && REG_RSM_HIBITS[1]) { // CH
					printf("ch");
				}
			}break;
			case REGISTER_D: {
				if(W && !REG_RSM_HIBITS[1]) { // DX
					printf("dx");
				} else if(!W && !REG_RSM_HIBITS[1]) { // DL
					printf("dl");
				} else if(W && REG_RSM_HIBITS[1]) { // SI
					printf("si");
				} else if(!W && REG_RSM_HIBITS[1]) { // DH
					printf("dh");
				}
			}break;
			case REGISTER_B: {
				if(W && !REG_RSM_HIBITS[1]) { // BX
					printf("bx");
				} else if(!W && !REG_RSM_HIBITS[1]) { // BL
					printf("bl");
				} else if(W && REG_RSM_HIBITS[1]) { // DI
					printf("di");
				} else if(!W && REG_RSM_HIBITS[1]) { // BH
					printf("bh");
				}
			}break;
			default: {
				printf("ILL");
			}break;
		}
		printf(" [%u %u (%u %u)] %u", D, W, REG_RSM_HIBITS[0], REG_RSM_HIBITS[1], LOBYTE(file[i]));
		printf("\n");
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
