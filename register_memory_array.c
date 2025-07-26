/* 	
	All possible values for REG and RM field. If RM is being used, each 
	entry in the array needs to be shifted by REG_RM_BITS.
*/
#define REG_RM_FIELD_OPTIONS_LENGTH 8
static uint8 REG_RM_FIELD_OPTIONS[REG_RM_FIELD_OPTIONS_LENGTH] = 
{
	0b00000000,
	0b00001000,
	0b00010000,
	0b00011000,

	0b00100000,
	0b00101000,
	0b00110000,
	0b00111000
};

/*
	Array of strings that correspond to the values in REG_RM_FIELD_OPTIONS.
	Intended use for this is to check a value against each entry in the
	aforementioned options array and keep the index of the match, then
	use that index to get the the string prepresentation of the name of the
	register. Note that to account for a wide operation when indexing, 
	you should add (W_FLAG_VALUE * REG_RM_OPTIONS_LENGTH) to the index to
	ensure you are getting the proper string.
*/
#define REGISTER_STRING_COUNT 8
static
char REG_FIELD_STRINGS[16][3] = 
{
	"al\0",
	"cl\0",
	"dl\0",
	"bl\0",
	"ah\0",
	"ch\0",
	"dh\0",
	"bh\0",

	"ax\0",
	"cx\0",
	"dx\0",
	"bx\0",
	"sp\0",
	"bp\0",
	"si\0",
	"di\0"
};
