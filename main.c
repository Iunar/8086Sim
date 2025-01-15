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
#define OPCODE_FIELD (u8)0b11111100
#define OPCODE_MOV (u8)0b10001000
#define OPCODE_LEA (u8)0b10001100

#define D_FIELD 0b00000010
#define W_FIELD 0b00000001
#define MOD_FIELD 0b11000000
#define REG_FIELD 0b00111000
#define RM_FIELD 0b00000111

// REGISTER_NL/NX (W=0/W=1)
#define REGISTER_A 0b00000000
#define REGISTER_C 0b00001000
#define REGISTER_D 0b00010000
#define REGISTER_B 0b00011000
#define REGISTER_HIGH 0b00100000

#define HIBYTE(n) ((u8)((n) >> 8))
#define LOBYTE(n) ((u8)(n))

u8 OPCODE = 0;
u8 D = 0;
u8 W = 0;
u8 MOD = 0;
u8 REG = 0;
u8 RM = 0;

// TODO: add error checking, too lazy
void* read_program(const char* path, u32* size_out);

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("Usage: decode <path/to/program>\n");
		return 0;
	}

	u32 file_size = 0;
	u8* file = (u8*)read_program(argv[argc-1], &file_size);
	printf("file_size: %u\n", file_size);
	for(int j = 0; j < file_size; j += 2) {
		u8 first_byte =  file[j];
		u8 second_byte = file[j + 1];

		//					Lo       Hi
		//printf("%u %u\n", first_byte, second_byte);

		// Extract info from first byte
		OPCODE = first_byte & OPCODE_FIELD;
		D = first_byte & D_FIELD;
		W = first_byte & W_FIELD;

		// Extract info from second byte
		MOD = second_byte & MOD_FIELD;
		REG = second_byte & REG_FIELD;
		RM = second_byte & RM_FIELD;

		// only decoding register to register instructions for now.
		//assert(MOD == MOD_FIELD);

		// Print opcode
		switch(OPCODE) {
			case OPCODE_MOV: {
				printf("mov ");
			}break;
			default:
				printf("ill ");
		}

		// Extract register operands
		u8 dst_register = 0;
		u8 src_register = 0;
		if(!D) {
			// rm contains dest, reg contains src
			dst_register = (second_byte & RM_FIELD) << 3;
			src_register = second_byte & REG_FIELD;
		} else {
			// reg contains dest, reg contains src
			dst_register = second_byte & REG_FIELD;
			src_register = (second_byte & RM_FIELD) << 3;
		}

		// print dest
		//printf("\n%u\n%u\n", dst_register, src_register);
		u8 regs[2] = { dst_register, src_register };
		for(int i = 0; i < 2; i++) {
			if(W) {
				switch(regs[i]) {
					case REGISTER_A: {
						printf("ax");
					}break;
					case (REGISTER_A | REGISTER_HIGH): {
						printf("sp");
					}break;
					case REGISTER_C: {
						printf("cx");
					}break;
					case (REGISTER_C | REGISTER_HIGH): {
						printf("bp");
					}break;
					case REGISTER_D: {
						printf("dx");
					}break;
					case (REGISTER_D | REGISTER_HIGH): {
						printf("si");
					}break;
					case REGISTER_B: {
						printf("bx");
					}break;
					case (REGISTER_B | REGISTER_HIGH): {
						printf("di");
					}break;
					default: {
						printf("ill");
					}break;
				}
			} else {
				switch(regs[i]) {
					case REGISTER_A: {
						printf("al");
					}break;
					case (REGISTER_A | REGISTER_HIGH): {
						printf("ah");
					}break;
					case REGISTER_C: {
						printf("cl");
					}break;
					case (REGISTER_C | REGISTER_HIGH): {
						printf("ch");
					}break;
					case REGISTER_D: {
						printf("dl");
					}break;
					case (REGISTER_D | REGISTER_HIGH): {
						printf("dh");
					}break;
					case REGISTER_B: {
						printf("bl");
					}break;
					case (REGISTER_B | REGISTER_HIGH): {
						printf("bh");
					}break;
					default: {
						printf("ill");
					}break;
				}
			}
			if(i == 0) {
				printf(", ");
			}
		}
		printf("\n");
	}

	free(file);
	return 0;
}

void* read_program(const char* path, u32* size_out) {
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
	return res;
}
