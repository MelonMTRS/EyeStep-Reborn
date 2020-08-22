# EyeStep-Reborn
Full and compact intel x86 disassembler + assembler

# About
This is completely original, and table-based from the resources provided at:
http://ref.x86asm.net/coder32.html

I ask that you give credit in some way for using this in your projects,
but that credit can be as simple as leaving my comments in this code :p

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
opmode - this is the mode, or, type of operand. There are many of these. DOCS COMING SOON
reg - table of registers in this operand (any bit size)
mul - multiplier used in SIB byte
rel8/rel16/rel32 - the relative offset used in this operand --- call sub_011C0D20 (...rel32 would be 011C0D20)
imm8/imm16/imm32 - the offset value used in this operand --- mov eax,[ebx+0120CDD0]
disp8/disp16/disp32 - the  constant value used in this operand --- mov eax,DEADBEEF





