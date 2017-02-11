# 8051Disassembler
Open source disassembler custom built to disassemble SiLabs C8051F530 code from Tesla BMS

There weren't any really good open source 8051 disassemblers. This one follows branches and disassembles as it goes.
This yields a better picture of the code. A straight through disassemble without following branches can accidentally interpret
data as code. This disassembler won't do that. At the moment it is very specifically targetted toward the Tesla BMS binary as
it does not have the ability to follow "JMP @A+DPTR" instructions which are most certainly used in the BMS code. So, the jump
tables which use that instruction have been manually decoded and those branches have been placed into the code.

With some limited changes this code could be easily used for other 8051 based processors.
