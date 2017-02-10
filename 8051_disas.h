#ifndef R2_8051_DISASM_H
#define R2_8051_DISASM_H

#define boolean uint8_t
#define false 0
#define true 1

typedef struct {
    const char *name;   //static portion of the decoded instruction
    int length;         //# of bytes this instruction spans
    int arg1;           //Type of the first argument
    int arg2;           //Type of the second argument
    boolean isJump;        //Does this instruction always jump
    boolean isCondJump;    //Does this instruction maybe branch?
} Instruct8051;

typedef struct {
	const char *name;
	int length;
	int32_t arg1;
    int arg1Type;
    int32_t arg2;
    int arg2Type;
    boolean isJump;
    boolean isCondJump;
	uint32_t addr;
} r_8051_op;

enum {
	NONE = 0,
	ADDR11 = 1,  // 8 bits from argument + 3 high bits from opcode
	ADDR16 = 2,  // A 16-bit address destination. Used by LCALL and LJMP
	DIRECT = 3,  // An internal data RAM location (0-127) or SFR (128-255).
    IMMED = 4,   // Immediate value (literal) - 8 Bit
    IMMED16 = 5, // Immediate Value (16 bit)
    BIT = 6,     // A bit within one of the bitfield bytes
	OFFSET = 7,  // same as direct?
    A = 8,       // Accum directly. Used only for second arg for a few ops
    C = 9,       // C
	IR0 = 10,    // @R0
	IR1 = 11,    // @R1
	R0 = 12,     // R0
	R1 = 13,     // R1
	R2 = 14,     // R2
	R3 = 15,     // R3
	R4 = 16,     // R4
	R5 = 17,     // R5
	R6 = 18,     // R6
	R7 = 19      // R7
};

r_8051_op r_8051_disasm(const unsigned char *buf, int len, uint32_t addr, char *outString, int outLen);

#endif /* 8051_DISASM_H */

