/* 
Structure of an 8086 instruction
     Register destination location
      |
      | Wide operation
      |   |
      |   |
      |   |      +-----------------[Mode][2bits]> Operation mode
      |   |      |
      |   |      |  +--------------[Register][3bits]> Register used in operation
      |   |      |  |
      +--+|      |  |  +-----------[Register/Memory][3bits]> Encodes register or
 +-------||-+ +--|--|--|-+                                   memory.
 | XXXXXXDW | | MODREGRM |
 +-|----|---+ +----------+
   +----+
   |
   |
   |
 Opcode
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define true 1
#define false 0
typedef uint8 bool;

#define assert(expr, fmt...)\
{\
	if(!(expr))\
	{\
		fprintf(stderr, fmt);\
		abort();\
	}\
}

#define OPCODE_MOV 	0b10001000
#define OPCODE_MASK 	0b11111100
#define D_FLAG_MASK 	0b00000010
#define W_FLAG_MASK 	0b00000001
#define MOD_MASK 	0b11000000
#define REG_MASK 	0b00111000
#define RM_MASK 	0b00000111
#define REG_RM_BITS 	3 // Number of bits for each the REG and the RM fields

#define MOD_MEMORY 	0b00000000
#define MOD_MEMORY_8 	0b01000000
#define MOD_MEMORY_16 	0b10000000
#define MOD_REGISTER 	0b11000000

#include "register_memory_array.c"

static void*
ReadFileRaw(const char* path);

void
PrintByte(uint8 Byte);

void
PrintOpcode(uint8 HiByte);

void
PrintInstruction(uint8 Instruction[6]);

void
PrintInstructionDetailed(uint8 Instruction[6]);

void
DecodeInstruction(uint16 Instruction, uint8 Result[6]);

static uint32 ProgramLength = 0;

int
main()
{
	void* InstructionsRaw =
		ReadFileRaw("single_register_mov");
	uint8* InstructionStream = (uint8*)InstructionsRaw;

	uint16 CurrentInstruction = 0;
	uint8  Instruction[6] = { 0 };
	for(	uint32 i = 0;
		i < ProgramLength;
		i += 2)
	{
		CurrentInstruction = 0;
		CurrentInstruction |= ((uint16)InstructionStream[i]) << 8;
		CurrentInstruction |= (uint16)(InstructionStream[i + 1]);

		DecodeInstruction(CurrentInstruction, Instruction);
		PrintInstruction(Instruction);
	}

	free(InstructionsRaw);
	return 0;
}

static void*
ReadFileRaw(const char* Path)
{
	void* Result = 0;

	FILE* FilePointer = fopen(Path, "r");
	assert(FilePointer, "Failed to open file %s\n", Path);

	assert(fseek(FilePointer, 0, SEEK_END) != -1,
		"Failed to seek in file.\n");

	long FileSize = ftell(FilePointer);
	assert(FileSize != -1, "Ftell failed.\n");

	rewind(FilePointer);

	Result = malloc(FileSize);
	assert(Result, "Failed to allocate bytes for file contents.\n");

	fread(Result, FileSize, 1, FilePointer);
	assert(ferror(FilePointer) == 0, "fread failed.\n");

	fclose(FilePointer);

	ProgramLength = FileSize;
	return Result;
}

void
PrintByte(uint8 Byte)
{
	printf("%c%c%c%c%c%c%c%c",
		((Byte & 0b10000000) ? '1' : '0'),
		((Byte & 0b01000000) ? '1' : '0'),
		((Byte & 0b00100000) ? '1' : '0'),
		((Byte & 0b00010000) ? '1' : '0'),
		((Byte & 0b00001000) ? '1' : '0'),
		((Byte & 0b00000100) ? '1' : '0'),
		((Byte & 0b00000010) ? '1' : '0'),
		((Byte & 0b00000001) ? '1' : '0'));
}

void
PrintOpcode(uint8 HiByte)
{
	printf("%c%c%c%c%c%c",
		((HiByte & 0b10000000) ? '1' : '0'),
		((HiByte & 0b01000000) ? '1' : '0'),
		((HiByte & 0b00100000) ? '1' : '0'),
		((HiByte & 0b00010000) ? '1' : '0'),
		((HiByte & 0b00001000) ? '1' : '0'),
		((HiByte & 0b00000100) ? '1' : '0'));
}

/*
void
PrintInstruction(
	uint8 Opcode, uint8 DFlag, uint8 WFlag, 
	uint8 Mod, uint8 Reg, uint8 Rm); */
void
PrintInstruction(uint8 Instruction[6])
{
	uint8 Opcode 	= Instruction[0];
	uint8 DFlag 	= Instruction[1];
	uint8 WFlag 	= Instruction[2];
	uint8 Mod 	= Instruction[3];
	uint8 Reg 	= Instruction[4];
	uint8 Rm 	= Instruction[5];


	/* Print Opcode */
	switch(Opcode)
	{
		case OPCODE_MOV:
		{
			printf("mov ");
		} break;
		//case :
		//{
		//} break;
		default:
		{
			assert(false, "UNIMPLEMENTED OR ILLEGAL OPCODE\n");
		} break;
	}

	/* Print */
	char* Destination = REG_FIELD_STRINGS[Reg + 
			(WFlag * REG_RM_FIELD_OPTIONS_LENGTH)];
	char* Source = 0;
	if(Mod == MOD_REGISTER)
	{
		Source = REG_FIELD_STRINGS[Rm + 
				(WFlag * REG_RM_FIELD_OPTIONS_LENGTH)];
	}
	else
	{
		assert(false, "MOD != MOD_REGISTER234\n");
	}

	if(!DFlag)
	{
		char* tmp = Source;
		Source = Destination;
		Destination = tmp;
	}

	printf("%s, %s\n", Destination, Source);
}

void
PrintInstructionDetailed(uint8 Instruction[6])
{
	uint8 Opcode 	= Instruction[0];
	uint8 DFlag 	= Instruction[1];
	uint8 WFlag 	= Instruction[2];
	uint8 Mod 	= Instruction[3];
	uint8 Reg 	= Instruction[4];
	uint8 Rm 	= Instruction[5];

	/* Print the Opcode */
	switch(Opcode)
	{
		case OPCODE_MOV:
		{
			printf("%-12s :mov\n", "OPCODE");
		} break;
		//case :
		//{
		//} break;
		default:
		{
			assert(false, "UNIMPLEMENTED OR ILLEGAL OPCODE\n");
		} break;
	}

	/* Print DFlag */
	printf("%-12s :%u\n", "D", DFlag);

	/* Print WFlag */
	printf("%-12s :%u\n", "W", WFlag);

	/* Print Mod */
	switch(Mod)
	{
		case MOD_MEMORY:
		{
			printf("%-12s :%s\n", "MOD", "Memory, no displacement");
		} break;
		case MOD_MEMORY_8:
		{
			printf("%-12s :%s\n", 
				"MOD", "Memory, 8-bit displacement");
		} break;
		case MOD_MEMORY_16:
		{
			printf("%-12s :%s\n", 
				"MOD", "Memory, 16-bit displacement");
		} break;
		case MOD_REGISTER:
		{
			printf("%-12s :%s\n", 
				"MOD", "Register");
		} break;
		//case :
		//{
		//} break;
		default:
		{
			assert(false, "UNIMPLEMENTED OR ILLEGAL OPCODE\n");
		} break;
	}
	
	/* Print Reg */
	printf("%-12s :%s\n", "REG", 
		REG_FIELD_STRINGS[Reg +
			(WFlag * REG_RM_FIELD_OPTIONS_LENGTH)]);

	/* Print Rm */
	printf("%-12s :%s\n", "RM", 
		REG_FIELD_STRINGS[Rm  +
			(WFlag * REG_RM_FIELD_OPTIONS_LENGTH)]);
}

void
DecodeInstruction(uint16 Instruction, uint8 Result[6])
{
	uint8 HiByte = (uint8)(Instruction >> 8);
	uint8 LoByte = (uint8)(Instruction);

	/* OPCODE FIELD */
	uint8 OPCODE = HiByte & OPCODE_MASK;

	/* D FIELD */
	uint8 D_FLAG = HiByte & D_FLAG_MASK;


	/* W FIELD */
	uint8 W_FLAG = HiByte & W_FLAG_MASK;

	/* MOD FIELD */
	uint8 MOD = LoByte & MOD_MASK;


	/* REG FIELD */
	uint8 REG = 255;
	for(	uint32 i = 0;
		i < REG_RM_FIELD_OPTIONS_LENGTH;
		i++)
	{
		if((LoByte & REG_MASK) == REG_RM_FIELD_OPTIONS[i])
		{
			REG = i;
		}
	}
	assert(REG < REGISTER_STRING_COUNT, "UNKNOWN REGISTER\n"); 

	/* RM FIELD */
	uint8 RM = 255;
	if(MOD == MOD_REGISTER)
	{
		for(	int i = 0;
			i < REG_RM_FIELD_OPTIONS_LENGTH;
			i++)
		{
			if((LoByte & RM_MASK) == 
				(REG_RM_FIELD_OPTIONS[i] >> REG_RM_BITS))
			{
				RM = i;
			}
		}
	}
	assert(RM < REGISTER_STRING_COUNT, "UNKNOWN RM\n");

	Result[0] = OPCODE;
	Result[1] = D_FLAG;
	Result[2] = W_FLAG;
	Result[3] = MOD;
	Result[4] = REG;
	Result[5] = RM;
}
