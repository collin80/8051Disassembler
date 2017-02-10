#if 0

http://www.keil.com/support/man/docs/is51/is51_instructions.htm
http://www.keil.com/support/man/docs/is51/is51_opcodes.htm

// TODO: extend support for 251

The classic 8051 provides 4 register banks of 8 registers each.
These register banks are mapped into the DATA memory area at
address 0 - 0x1F. In addition the CPU provides a 8-bit A
(accumulator) and B register and a 16-bit DPTR (data pointer)
for addressing XDATA and CODE memory. These registers are also
mapped into the SFR space as special function registers.

|-----------------------|
 r0 r1 r2 r3 r4 r5 r6 r7  0x00
 r0 r1 r2 r3 r4 r5 r6 r7  0x08
 r0 r1 r2 r3 r4 r5 r6 r7  0x10
 r0 r1 r2 r3 r4 r5 r6 r7  0x18

A = acumulator
B = general purpose
DPTR = 16 bit pointer to data

PSW1 - status word register

Bit 7	Bit 6	Bit 5	Bit 4	Bit 3	Bit 2	Bit 1	Bit 0
CY	AC	N	RS1	RS0	OV	Z	-

The following table describes the status bits in the PSW:

RS1 RS0 Working Register Bank and Address
0 0 Bank0 (D:0x00 - D:0x07)
0 1 Bank1 (D:0x08 - D:0x0F)
1 0 Bank2 (D:0x10 - D:0x17)
1 1 Bank3 (D:0x18H - D:0x1F)

#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "8051_disas.h"

//all 256 op codes
static Instruct8051 OpCodes[] =
{
//   name  len  arg1  arg1  jmp   cond-jmp
    {"NOP", 1, NONE, NONE, false, false},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"LJMP ", 3, ADDR16, NONE, true, false},
    {"RR A", 1, NONE, NONE, false, false},
    {"INC A", 1, NONE, NONE, false, false},
    {"INC ", 2, DIRECT, NONE, false, false},
    {"INC @R0", 1, NONE, NONE, false, false},
    {"INC @R1", 1, NONE, NONE, false, false},
    {"INC R0", 1, NONE, NONE, false, false},
    {"INC R1", 1, NONE, NONE, false, false},
    {"INC R2", 1, NONE, NONE, false, false},
    {"INC R3", 1, NONE, NONE, false, false},
    {"INC R4", 1, NONE, NONE, false, false},
    {"INC R5", 1, NONE, NONE, false, false},
    {"INC R6", 1, NONE, NONE, false, false},
    {"INC R7", 1, NONE, NONE, false, false},      //F
    {"JBC ", 3, BIT, OFFSET, false, true},
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"LCALL ", 3, ADDR16, NONE, false, true},
    {"RRC A", 1, NONE, NONE, false, false},
    {"DEC A", 1, NONE, NONE, false, false},
    {"DEC ", 2, DIRECT, NONE, false, false},
    {"DEC @R0", 1, NONE, NONE, false, false},
    {"DEC @R1", 1, NONE, NONE, false, false},
    {"DEC R0", 1, NONE, NONE, false, false},
    {"DEC R1", 1, NONE, NONE, false, false},
    {"DEC R2", 1, NONE, NONE, false, false},
    {"DEC R3", 1, NONE, NONE, false, false},
    {"DEC R4", 1, NONE, NONE, false, false},
    {"DEC R5", 1, NONE, NONE, false, false},
    {"DEC R6", 1, NONE, NONE, false, false},
    {"DEC R7", 1, NONE, NONE, false, false},   //1F
    {"JB ", 3, BIT, OFFSET, false, true},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"RET", 1, NONE, NONE, false, false},
    {"RL A", 1, NONE, NONE, false, false},    
    {"ADD A, ", 2, IMMED, NONE, false, false},
    {"ADD A, ", 2, DIRECT, NONE, false, false},
    {"ADD A, @R0", 1, NONE, NONE, false, false},
    {"ADD A, @R1", 1, NONE, NONE, false, false},
    {"ADD A, R0", 1, NONE, NONE, false, false},
    {"ADD A, R1", 1, NONE, NONE, false, false},
    {"ADD A, R2", 1, NONE, NONE, false, false},
    {"ADD A, R3", 1, NONE, NONE, false, false},
    {"ADD A, R4", 1, NONE, NONE, false, false},
    {"ADD A, R5", 1, NONE, NONE, false, false},
    {"ADD A, R6", 1, NONE, NONE, false, false},
    {"ADD A, R7", 1, NONE, NONE, false, false},   //2F
    {"JNB ", 3, BIT, OFFSET, false, true},
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"RETI", 1, NONE, NONE, false, false},
    {"RLC A", 1, NONE, NONE, false, false},
    {"ADDC A, ", 2, IMMED, NONE, false, false},
    {"ADDC A, ", 2, DIRECT, NONE, false, false},
    {"ADDC A, @R0", 1, NONE, NONE, false, false},
    {"ADDC A, @R1", 1, NONE, NONE, false, false},
    {"ADDC A, R0", 1, NONE, NONE, false, false},
    {"ADDC A, R1", 1, NONE, NONE, false, false},
    {"ADDC A, R2", 1, NONE, NONE, false, false},
    {"ADDC A, R3", 1, NONE, NONE, false, false},
    {"ADDC A, R4", 1, NONE, NONE, false, false},
    {"ADDC A, R5", 1, NONE, NONE, false, false},
    {"ADDC A, R6", 1, NONE, NONE, false, false},
    {"ADDC A, R7", 1, NONE, NONE, false, false},   //3F
    {"JC ", 2, OFFSET, NONE, false, true},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"ORL ", 2, DIRECT, A, false, false},
    {"ORL ", 3, DIRECT, IMMED, false, false},
    {"ORL A, ", 2, IMMED, NONE, false, false},
    {"ORL A, ", 2, DIRECT, NONE, false, false},
    {"ORL A, @R0", 1, NONE, NONE, false, false},
    {"ORL A, @R1", 1, NONE, NONE, false, false},
    {"ORL A, R0", 1, NONE, NONE, false, false},
    {"ORL A, R1", 1, NONE, NONE, false, false},
    {"ORL A, R2", 1, NONE, NONE, false, false},
    {"ORL A, R3", 1, NONE, NONE, false, false},
    {"ORL A, R4", 1, NONE, NONE, false, false},
    {"ORL A, R5", 1, NONE, NONE, false, false},
    {"ORL A, R6", 1, NONE, NONE, false, false},
    {"ORL A, R7", 1, NONE, NONE, false, false},    //4F
    {"JNC ", 2, OFFSET, NONE, false, true},    
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"ANL ", 2, DIRECT, A, false, false},
    {"ANL ", 3, DIRECT, IMMED, false, false},
    {"ANL A, ", 2, IMMED, NONE, false, false},
    {"ANL A, ", 2, DIRECT, NONE, false, false},
    {"ANL A, @R0", 1, NONE, NONE, false, false},
    {"ANL A, @R1", 1, NONE, NONE, false, false},
    {"ANL A, R0", 1, NONE, NONE, false, false},
    {"ANL A, R1", 1, NONE, NONE, false, false},
    {"ANL A, R2", 1, NONE, NONE, false, false},
    {"ANL A, R3", 1, NONE, NONE, false, false},
    {"ANL A, R4", 1, NONE, NONE, false, false},
    {"ANL A, R5", 1, NONE, NONE, false, false},
    {"ANL A, R6", 1, NONE, NONE, false, false},
    {"ANL A, R7", 1, NONE, NONE, false, false},    //5F
    {"JZ ", 2, OFFSET, NONE, false, true},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"XRL ", 2, DIRECT, A, false, false},
    {"XRL ", 3, DIRECT, IMMED, false, false},
    {"XRL A, ", 2, IMMED, NONE, false, false},
    {"XRL A, ", 2, DIRECT, NONE, false, false},
    {"XRL A, @R0", 1, NONE, NONE, false, false},
    {"XRL A, @R1", 1, NONE, NONE, false, false},
    {"XRL A, R0", 1, NONE, NONE, false, false},
    {"XRL A, R1", 1, NONE, NONE, false, false},
    {"XRL A, R2", 1, NONE, NONE, false, false},
    {"XRL A, R3", 1, NONE, NONE, false, false},
    {"XRL A, R4", 1, NONE, NONE, false, false},
    {"XRL A, R5", 1, NONE, NONE, false, false},
    {"XRL A, R6", 1, NONE, NONE, false, false},
    {"XRL A, R7", 1, NONE, NONE, false, false},  //6F
    {"JNZ ", 2, OFFSET, NONE, false, true},
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"ORL C, ", 2, BIT, NONE, false, false},
    {"JMP @A+DPTR", 1, NONE, NONE, true, false},
    {"MOV A, ", 2, IMMED, NONE, false, false},
    {"MOV ", 3, DIRECT, IMMED, false, false},
    {"MOV @R0, ", 2, IMMED, NONE, false, false},
    {"MOV @R1, ", 2, IMMED, NONE, false, false},
    {"MOV R0, ", 2, IMMED, NONE, false, false},
    {"MOV R1, ", 2, IMMED, NONE, false, false},
    {"MOV R2, ", 2, IMMED, NONE, false, false},
    {"MOV R3, ", 2, IMMED, NONE, false, false},
    {"MOV R4, ", 2, IMMED, NONE, false, false},
    {"MOV R5, ", 2, IMMED, NONE, false, false},
    {"MOV R6, ", 2, IMMED, NONE, false, false},
    {"MOV R7, ", 2, IMMED, NONE, false, false},  //7F
    {"SJMP ", 2, OFFSET, NONE, true, false},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"ANL C, ", 2, BIT, NONE, false, false},
    {"MOVC A, @A+PC", 1, NONE, NONE, false, false},
    {"DIV AB", 1, NONE, NONE, false, false},
    {"MOV ", 3, DIRECT, DIRECT, false, false},
    {"MOV ", 2, DIRECT, IR0, false, false},
    {"MOV ", 2, DIRECT, IR1, false, false},
    {"MOV ", 2, DIRECT, R0, false, false},
    {"MOV ", 2, DIRECT, R1, false, false},
    {"MOV ", 2, DIRECT, R2, false, false},
    {"MOV ", 2, DIRECT, R3, false, false},
    {"MOV ", 2, DIRECT, R4, false, false},
    {"MOV ", 2, DIRECT, R5, false, false},
    {"MOV ", 2, DIRECT, R6, false, false},
    {"MOV ", 2, DIRECT, R7, false, false},      //8F
    {"MOV DPTR, ", 3, IMMED16, NONE, false, false},
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"MOV ", 2, BIT, C, false, false},
    {"MOVC A, @A+DPTR", 1, NONE, NONE, false, false},
    {"SUBB A, ", 2, IMMED, NONE, false, false},
    {"SUBB A, ", 2, DIRECT, NONE, false, false},
    {"SUBB A, @R0", 1, NONE, NONE, false, false},
    {"SUBB A, @R1", 1, NONE, NONE, false, false},
    {"SUBB A, R0", 1, NONE, NONE, false, false},
    {"SUBB A, R1", 1, NONE, NONE, false, false},
    {"SUBB A, R2", 1, NONE, NONE, false, false},
    {"SUBB A, R3", 1, NONE, NONE, false, false},
    {"SUBB A, R4", 1, NONE, NONE, false, false},
    {"SUBB A, R5", 1, NONE, NONE, false, false},
    {"SUBB A, R6", 1, NONE, NONE, false, false},
    {"SUBB A, R7", 1, NONE, NONE, false, false}, //9F
    {"ORL C, ", 2, BIT, NONE, false, false},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"MOV C, ", 2, BIT, NONE, false, false},
    {"INC DPTR", 1, NONE, NONE, false, false},
    {"MUL AB", 1, NONE, NONE, false, false},
    {"RESERVED", 1, NONE, NONE, false, false},
    {"MOV @R0, ", 2, DIRECT, NONE, false, false},
    {"MOV @R1, ", 2, DIRECT, NONE, false, false},
    {"MOV R0, ", 2, DIRECT, NONE, false, false},
    {"MOV R1, ", 2, DIRECT, NONE, false, false},
    {"MOV R2, ", 2, DIRECT, NONE, false, false},
    {"MOV R3, ", 2, DIRECT, NONE, false, false},
    {"MOV R4, ", 2, DIRECT, NONE, false, false},
    {"MOV R5, ", 2, DIRECT, NONE, false, false},
    {"MOV R6, ", 2, DIRECT, NONE, false, false},
    {"MOV R7, ", 2, DIRECT, NONE, false, false},  //AF
    {"ANL C, ", 2, BIT, NONE, false, false},
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"CPL ", 2, BIT, NONE, false, false},
    {"CPL C", 1, NONE, NONE, false, false},
    {"CJNE A, ", 3, IMMED, OFFSET, false, true},
    {"CJNE A, ", 3, DIRECT, OFFSET, false, true},
    {"CJNE @R0, ", 3, IMMED, OFFSET, false, true},
    {"CJNE @R1, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R0, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R1, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R2, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R3, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R4, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R5, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R6, ", 3, IMMED, OFFSET, false, true},
    {"CJNE R7, ", 3, IMMED, OFFSET, false, true},  //BF
    {"PUSH ", 2, DIRECT, NONE, false, false},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"CLR ", 2, BIT, NONE, false, false},
    {"CLR C", 1, NONE, NONE, false, false},
    {"SWAP A", 1, NONE, NONE, false, false},
    {"XCH A, ", 2, DIRECT, NONE, false, false},
    {"XCH A, @R0", 1, NONE, NONE, false, false},
    {"XCH A, @R1", 1, NONE, NONE, false, false},
    {"XCH A, R0", 1, NONE, NONE, false, false},
    {"XCH A, R1", 1, NONE, NONE, false, false},
    {"XCH A, R2", 1, NONE, NONE, false, false},
    {"XCH A, R3", 1, NONE, NONE, false, false},
    {"XCH A, R4", 1, NONE, NONE, false, false},
    {"XCH A, R5", 1, NONE, NONE, false, false},
    {"XCH A, R6", 1, NONE, NONE, false, false},
    {"XCH A, R7", 1, NONE, NONE, false, false},    //CF
    {"POP ", 2, DIRECT, NONE, false, false},
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"SETB ", 2, BIT, NONE, false, false},
    {"SETB C", 1, NONE, NONE, false, false},
    {"DA A", 1, NONE, NONE, false, false},
    {"DJNZ ", 3, DIRECT, OFFSET, false, true},
    {"XCHD A, @R0", 1, NONE, NONE, false, false},
    {"XCHD A, @R1", 1, NONE, NONE, false, false},
    {"DJNZ R0, ", 2, OFFSET, NONE, false, true},
    {"DJNZ R1, ", 2, OFFSET, NONE, false, true},
    {"DJNZ R2, ", 2, OFFSET, NONE, false, true},
    {"DJNZ R3, ", 2, OFFSET, NONE, false, true},
    {"DJNZ R4, ", 2, OFFSET, NONE, false, true},
    {"DJNZ R5, ", 2, OFFSET, NONE, false, true},
    {"DJNZ R6, ", 2, OFFSET, NONE, false, true},
    {"DJNZ R7, ", 2, OFFSET, NONE, false, true},    //DF
    {"MOVX A, @DPTR", 1, NONE, NONE, false, false},
    {"AJMP ", 2, ADDR11, NONE, true, false},
    {"MOVX, A, @R0", 1, NONE, NONE, false, false},
    {"MOVX, A, @R1", 1, NONE, NONE, false, false},
    {"CLR A", 1, NONE, NONE, false, false},
    {"MOV A, ", 2, DIRECT, NONE, false, false},
    {"MOV A, @R0", 1, NONE, NONE, false, false},
    {"MOV A, @R1", 1, NONE, NONE, false, false},
    {"MOV A, R0", 1, NONE, NONE, false, false},
    {"MOV A, R1", 1, NONE, NONE, false, false},
    {"MOV A, R2", 1, NONE, NONE, false, false},
    {"MOV A, R3", 1, NONE, NONE, false, false},
    {"MOV A, R4", 1, NONE, NONE, false, false},
    {"MOV A, R5", 1, NONE, NONE, false, false},
    {"MOV A, R6", 1, NONE, NONE, false, false},
    {"MOV A, R7", 1, NONE, NONE, false, false},      //EF
    {"MOVX @DPTR, A", 1, NONE, NONE, false, false},
    {"ACALL ", 2, ADDR11, NONE, false, true},
    {"MOVX @R0, A", 1, NONE, NONE, false, false},
    {"MOVX @R1, A", 1, NONE, NONE, false, false},
    {"CPL A", 1, NONE, NONE, false, false},
    {"MOV ", 2, DIRECT, A, false, false},
    {"MOV @R0, A", 1, NONE, NONE, false, false},
    {"MOV @R1, A", 1, NONE, NONE, false, false},
    {"MOV R0, A", 1, NONE, NONE, false, false},
    {"MOV R1, A", 1, NONE, NONE, false, false},
    {"MOV R2, A", 1, NONE, NONE, false, false},
    {"MOV R3, A", 1, NONE, NONE, false, false},
    {"MOV R4, A", 1, NONE, NONE, false, false},
    {"MOV R5, A", 1, NONE, NONE, false, false},
    {"MOV R6, A", 1, NONE, NONE, false, false},
    {"MOV R7, A", 1, NONE, NONE, false, false}    //FF
};

//Starting at 0x80 and going through 0xFF these are the SFR's for a C8051F530 from Silicon Labs
const char *SFR[] = 
{
"P0", "SP","DPL","DPH","x84","x85","x86","PCON",  //80-87
"TCON", "TMOD","TL0","TL1","TH0","TH1","CKCON","PSCTL",  //88-8F
"P1", "x91","LINADDR","LINDATA","x94","LINCF","x96","x97",  //90-87
"SCON", "SBUF","x9A","CPTCN","x9C","CPTMD","x9E","CPTMX",  //98-8F
"xA0", "SPICFG","SPICKR","SPIDAT","P0MDOUT","P1MDOUT","x86","x87",  //A0-A7
"IE", "CLKSEL","xAA","xAB","xAC","xAD","xAE","xAF",  //A8-AF
"OSCIFIN", "OSCXCN","OSCICN","OSCICL","xB4","xB5","xB6","FLKEY",  //B0-B7
"IP", "xB9","ADCTK","ADCMX","ADCCF","ADCL","ADC","P1MASK",  //B8-BF
"xC0", "xC1","xC2","ADCGTL","ADCGTH","ADCLTL","ADCLTH","P0MASK",  //C0-C7
"TMP2CN", "REGCN","TMR2RLL","TMR2RLH","TMP2L","TMR2H","x8E","P1MAT",  //C8-CF
"PSW", "REFCN","xD2","xD3","P0SKIP","P1SKIP","xD6","P0MAT",  //D0-D7
"PCACN", "PCAMD","PCACPM0","PCACPM1","PCACPM2","xDD","xDE","xDF",  //D8-DF
"ACC", "XBR0","XBR1","xE3","IT01CF","xE5","EIE1","xE7",  //E0-E7
"ADCCN", "PCACPL1","PCACPH1","PCACPL2","PCACPH2","xED","xEE","RSTSRC",  //E8-EF
"B", "P0MDIN","P1MDIN","xF3","xF4","xF5","EIP1","xF7",  //F0-F7
"SPICN", "PCAL","PCAH","PCACPL0","PCACPH0","xFD","xFE","VDDMON",  //F8-FF
};

unsigned char codeSpace[1024]; //bitfield 8192 wide - each bit is 1 if ROM byte is code, 0 if it is data
unsigned char disasm[8192][64]; //disassembled instructions with 64 bytes allocated for each byte
unsigned char fileBuff[8192];

int interpretArgument(int32_t *outInt, char *inBuffer, int opType, uint32_t address, int op)
{
    int32_t tempInt = 0;
    int offset = 0;
    int opSize = OpCodes[op].length;
    switch(opType)
    {
    case NONE:
        offset = 0;
        tempInt = 0;
        break;
    case ADDR11:
        tempInt = (((unsigned char)op & 0xE0) << 3) + (unsigned char)inBuffer[0];
        offset = 1;
        break;
    case ADDR16:
        tempInt = ((unsigned char)inBuffer[0] << 8) + (unsigned char)inBuffer[1];
        offset = 2;
        break;
	case DIRECT:
        tempInt = (unsigned char)inBuffer[0];
        offset = 1;
        break;
    case IMMED:
        tempInt = (unsigned char)inBuffer[0];
        offset = 1;
        break;
    case IMMED16:
        tempInt = ((unsigned char)inBuffer[0] << 8) + (unsigned char)inBuffer[1];
        offset = 2;
        break;
    case BIT:
        tempInt = (unsigned char)inBuffer[0];
        offset = 1;        
        break;
    case OFFSET: //offset is from the end of this current instruction
        tempInt = address + opSize + (signed char)(inBuffer[0]);
        offset = 1;
        break;
    default:
        tempInt = 0;
        offset = 0;
    }
    *outInt = tempInt;

    return offset;
}

void printArgument(int argType, int32_t value, char *outBuffer)
{
    switch(argType)
    {
    case NONE:        
        break;
    case ADDR11:       
        sprintf(outBuffer, "0x%04X", value);
        break;
    case ADDR16:
        sprintf(outBuffer, "0x%04X", value);
        break;
	case DIRECT:
        if (value < 0x80) sprintf(outBuffer, "X%02Xh", value);
        else strcpy(outBuffer, SFR[value - 0x80]);
        break;
    case IMMED:
        sprintf(outBuffer, "#0x%02X", value);
        break;
    case IMMED16:
        sprintf(outBuffer, "#0x%04X", value);
        break;
    case BIT:
        //the last 128 bits are in 80, 88, 90, 98 that is 0x80 and upward 8 at a time
        if (value < 0x80) //bits from bytes 0x20 to 0x2F
        {
            sprintf(outBuffer, "%X.%dh", 0x20 + (value / 8), value % 8);
        }
        else
        {
            sprintf(outBuffer, "%X.%dh", value & 0xF8, value & 0x07);
        }
        break;
    case OFFSET:
        sprintf(outBuffer, "0x%04X", value);
        break;
    case A:
        sprintf(outBuffer, "A");
        break;
    case C:
        sprintf(outBuffer, "C");
        break;
    case IR0:
        sprintf(outBuffer, "@R0");
        break;
    case IR1:
        sprintf(outBuffer, "@R1");
        break;
    case R0:
        sprintf(outBuffer, "R0");
        break;
    case R1:
        sprintf(outBuffer, "R1");
        break;
    case R2:
        sprintf(outBuffer, "R2");
        break;
    case R3:
        sprintf(outBuffer, "R3");
        break;
    case R4:
        sprintf(outBuffer, "R4");
        break;
    case R5:
        sprintf(outBuffer, "R5");
        break;
    case R6:
        sprintf(outBuffer, "R6");
        break;
    case R7:
        sprintf(outBuffer, "R7");
        break;
    }
}

r_8051_op r_8051_disasm(const unsigned char *buf, int bufLen, uint32_t addr, char *outString, int outLen) {
	char *out = NULL;
    char *arg1Str = NULL;
    char *arg2Str = NULL;
    int tempInt;
    int offset = 0;
    unsigned char opNum;
    r_8051_op op;

	if (outString && *outString && outLen > 10) {
		out = strdup (outString);
	} 
    else {
		outLen = 64;
		out = malloc (outLen);
		*out = 0;
	}

    arg1Str = malloc(10);
    arg2Str = malloc(10);
    *arg1Str = 0;
    *arg2Str = 0;

    opNum = buf[0];
    op.name = OpCodes[opNum].name;
    op.length = OpCodes[opNum].length;
    op.arg1Type = OpCodes[opNum].arg1;
    op.arg2Type = OpCodes[opNum].arg2;
    op.isJump = OpCodes[opNum].isJump;
    op.isCondJump = OpCodes[opNum].isCondJump;

    //printf("OpName: %s length: %i arg1t: %i arg2t: %i\n", op.name, op.length, op.arg1Type, op.arg2Type);

    offset = interpretArgument(&op.arg1, &buf[1], op.arg1Type, addr, opNum);
    offset = interpretArgument(&op.arg2, &buf[1 + offset], op.arg2Type, addr, opNum);

    if (op.arg1Type == ADDR11 || op.arg1Type == ADDR16 || op.arg1Type == OFFSET) op.addr = op.arg1;
    if (op.arg2Type == ADDR11 || op.arg2Type == ADDR16 || op.arg2Type == OFFSET) op.addr = op.arg2;

    printArgument(op.arg1Type, op.arg1, arg1Str);
    printArgument(op.arg2Type, op.arg2, arg2Str);
    
    if (arg2Str && *arg2Str) {
        sprintf(outString, "%s %s, %s", OpCodes[opNum].name, arg1Str, arg2Str);
    }
    else {
        sprintf(outString, "%s %s", OpCodes[opNum].name, arg1Str);
    }

    free (arg1Str);
    free (arg2Str);
    return op;
}

//given a starting byte and a length it marks all of those bytes as code bytes
void setCodeSpaceBits(int startByt, int length)
{
    int byt;
    int bit;
    for (int x = startByt; x < startByt + length; x++)
    {
        byt = x / 8;
        bit = x & 7;
        //printf("byt: %i bit: %i\n", byt, bit);
        codeSpace[byt] |= (1 << bit);
    }
}

//given a ROM byte, return true if it has been marked as code, false if unmarked / data
boolean getCodeSpaceBit(int byt)
{
    int contByt = byt / 8;
    int bit = byt & 7;
    if (codeSpace[contByt] & (1 << bit)) return true;
    return false;
}

void followCodePath(uint32_t startingAddr)
{
    r_8051_op currentOp;
    unsigned char outBuff[64];

    printf("Start of code path: %x\n", startingAddr);

    while (1 == 1)
    {
        //printf("About to interpret instruction at: %x\n", startingAddr);
        if (getCodeSpaceBit(startingAddr))
        { 
            printf("At address: %x tried to process a location we've already been to.\n", startingAddr);
            return; //quit processing if we go back over code that was already processed.
        }
        //decode the next instruction
        currentOp = r_8051_disasm(&fileBuff[startingAddr], 8192-startingAddr, startingAddr, outBuff, 64);
        //set all of its bytes as code containing bytes
        setCodeSpaceBits(startingAddr, currentOp.length);
        //also copy it's disassembled text version into the master instructions list
        strcpy(disasm[startingAddr], outBuff);
        printf("%s\n", outBuff);

        //if this instruction could possibly jump then take that jump
        if (currentOp.isCondJump || currentOp.isJump) followCodePath(currentOp.addr);

        //there are a variety of reasons we might quit going forward here.
        //perhaps this is a non-conditional jump. In that case we can't go anymore
        //or, perhaps we hit a RET or RETI instruction. So, check for all of the above
        if (currentOp.isJump || !strcmp("RET",currentOp.name) || !strcmp("RETI",currentOp.name)) 
        {
            printf("End of code path at: %x\n", startingAddr);
            return;
        }
              
        startingAddr += currentOp.length;
    }


}

int main(int argc, char *argv[]) {    
    FILE *inFile;
    FILE *outFile;

	if (argc < 2)
    {
        printf("\nListen dope, you need to provide a filename to load.\n");
        return -1;
    }

    memset(codeSpace, 0, 1024);
    memset(disasm, 0, 8192 * 64);

    outFile = fopen("outAsm.txt", "w");
    inFile = fopen(argv[1], "rb");
    fread(fileBuff, 8192, 1, inFile);

    /*
    after reading in the file grab the instruction at 0 which is the reset vector. This sends us on our way. Disassemble it
    */
    followCodePath(0);      //reset vector
    followCodePath(0x23);   //UART interrupt
    followCodePath(0x2B);   //Timer2 Interrupt

    //These six are jump table targets all in one jump table
    followCodePath(0xA1A);
    followCodePath(0xA5B);
    followCodePath(0x80A);
    followCodePath(0x9A7);
    followCodePath(0xB38);
    followCodePath(0xBCE);

    //Here's the targets of another jump table
    followCodePath(0xF27);
    followCodePath(0x10A2);
    followCodePath(0x1154);
    followCodePath(0x10B8);
    followCodePath(0x1175);
    followCodePath(0x11EB);
    followCodePath(0x1279);

    printf("\nCode Visualization:\n");

    boolean isCode;
    for (int y = 0; y < 128; y++)
    {   
        printf("|");
        for (int x = 0; x < 64; x++)
        {        
            isCode = getCodeSpaceBit((y * 64) + x);
            if (isCode) printf(".");
            else printf(" ");
        }
        printf("|%04X to %04X\n", y*64, (y*64 + 63));
    }

    //for each byte see if it encodes a disassembled instruction and dump it if so
    for (int x = 0; x < 8192; x++)
    {
        if(disasm[x][0])
        {
            fprintf(outFile, "0x%04X %s\n", x, disasm[x]);
        }
        else if (!getCodeSpaceBit(x))
        {
            fprintf(outFile, "0x%04X      0x%02X\n", x, fileBuff[x]);
        }
    }

    fclose(outFile);
    fclose(inFile);
	return 0;
}

