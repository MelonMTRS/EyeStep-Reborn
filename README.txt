# About

EyeStep is a full and compact intel x86 disassembler, with API for 
a complete run-time analysis of memory.

It's original and table-based from the resources provided at:
http://ref.x86asm.net/coder32.html

It features everything except for SIMD instructions
but I intend to add support for that and possibly x64
in the future.


# Usage

To use EyeStep you simply include eyestep.h where needed
use EyeStep::ReadInstruction(address) to interpret the approximate
x86 instruction at 'address'.
You can view the disassembly like so:

auto instr = EyeStep::ReadInstruction(0x12000000); // returns an EyeStep::inst object
std::cout << instr.data << std::endl;


The inst, or, instruction class, contains the following:

data - text translation of disassembly
len - length of instruction
pre_flags - OR'd flags for instruction prefixes
address - location in memory of this instruction (whatever you called ReadInstruction with)
operands - table of operands used

'operands' can contain up to 4 operands per instruction.
for example, mov eax,[ebx] contains 2 operands (source & destination)
so for source you would use operands[0].
For destination you would use operands[1].


The operand class contains the following:

opmode - this is the mode, or, type of operand. There are many of these. SEE BELOW***
reg - table of registers in this operand (any bit size)
mul - multiplier used in SIB byte
rel8/rel16/rel32 - the relative offset used in this operand --- Example: call 011C0D20 (...rel32 would be 011C0D20)
imm8/imm16/imm32 - the offset value used in this operand --- Example: mov eax,[ebx+0120CDD0]
disp8/disp16/disp32 - the constant value used in this operand --- Example: mov eax,DEADBEEF


*** some examples of opmodes are:

mov eax,[ebx+04] \/\/\/
operands[0].opmode = r16_32/r32 // this is what will be expected as the first operand's opmode
operands[1].opmode = r_m16_32/r_m32 // second operand's opcode

mov [ebx+04],eax \/\/\/
operands[0].opmode = r_m16_32/r_m32
operands[1].opmode = r16_32/r32

call 0110CBF0 \/\/\/
operands[0].opmode = rel32

mov eax,10000000 \/\/\/
operands[0].opmode = imm32 // (but disp32 gets set)
operands[0].disp32 = 10000000 // constant value

mov eax,[10000000] \/\/\/
operands[0].opmode = imm32
operands[0].imm32 = 10000000




# API / Utility library

eyestep_utility.h and eyestep_utility.cpp provide a vast range
of functions for reading/writing/processing memory in either a
DLL or remote application.

Documentation on its way

