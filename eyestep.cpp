#pragma warning(disable:4996)
#include <Windows.h>
#include "eyestep.h"

#define MOD_NOT_FIRST 255

// this could be cleaner
// maybe
namespace mnemonics
{
	const char* r8_names[] = 
	{
		"al",
		"cl",
		"dl",
		"bl",
		"ah",
		"ch",
		"dh",
		"bh"
	};

	const char* r16_names[] = 
	{
		"ax",
		"cx",
		"dx",
		"bx",
		"sp",
		"bp",
		"si",
		"di"
	};

	const char* r32_names[] = 
	{
		"eax",
		"ecx",
		"edx",
		"ebx",
		"esp",
		"ebp",
		"esi",
		"edi"
	};

	const char* r64_names[] = 
	{
		"rax",
		"rcx",
		"rdx",
		"rbx",
		"rsp",
		"rbp",
		"rsi",
		"rdi"
	};

	const char* rxmm_names[] = 
	{
		"xmm0",
		"xmm1",
		"xmm2",
		"xmm3",
		"xmm4",
		"xmm5",
		"xmm6",
		"xmm7"
	};

	const char* sreg_names[] = 
	{
		"es",
		"cs",
		"ss",
		"ds",
		"fs",
		"gs",
		"hs",
		"is"
	};

	const char* dr_names[] = // debug register
	{
		"dr0",
		"dr1",
		"dr2",
		"dr3",
		"dr4",
		"dr5",
		"dr6",
		"dr7"
	};

	const char* cr_names[] = // control register
	{
		"cr0",
		"cr1",
		"cr2",
		"cr3",
		"cr4",
		"cr5",
		"cr6",
		"cr7"
	};

	const char* st_names[] = // control register
	{
		"st(0)",
		"st(1)",
		"st(2)",
		"st(3)",
		"st(4)",
		"st(5)",
		"st(6)",
		"st(7)"
	};
}

const uint8_t multipliers[] = 
{
	0, 
	2, 
	4, 
	8 
};

uint32_t getm20(uint8_t byte) 
{
	return byte % 32;
}

uint32_t getm40(uint8_t byte) 
{
	return byte % 64;
}

uint32_t finalreg(uint8_t byte)
{
	return byte % 64 % 8;
}

uint32_t longreg(uint8_t byte)
{
	return byte % 64 / 8;
}

namespace EyeStep
{
	// This has been optimized so that the
	// prefix, two-mode byte, and opcode byte 
	// are all packed into the same string
	// 
	const std::vector<OP_INFO> OP_TABLE =
	{
		{ "00", "add", { r_m8, r8 },					"Add" },
		{ "01", "add", { r_m16_32, r16_32 },			"Add" },
		{ "02", "add", { r8, r_m8 },					"Add" },
		{ "03", "add", { r16_32, r_m16_32 },			"Add" },
		{ "04", "add", { AL, imm8 },					"Add" },
		{ "05", "add", { EAX, imm16_32 },				"Add" },
		{ "06", "push", { ES },							"Push Extra Segment onto the stack" },
		{ "07", "pop", { ES },							"Pop Extra Segment off of the stack" },
		{ "08", "or", { r_m8, r8 },						"Logical Inclusive OR" },
		{ "09", "or", { r_m16_32, r16_32 },				"Logical Inclusive OR" },
		{ "0A", "or", { r8, r_m8 },						"Logical Inclusive OR" },
		{ "0B", "or", { r16_32, r_m16_32 },				"Logical Inclusive OR" },
		{ "0C", "or", { AL, imm8 },						"Logical Inclusive OR" },
		{ "0D", "or", { EAX, imm16_32 },				"Logical Inclusive OR" },
		{ "0E", "push", { CS },							"Push Code Segment onto the stack" },
		{ "0F+00+m0", "sldt", { r_m16_32 },				"Store Local Descriptor Table Register" },
		{ "0F+00+m1", "str", { r_m16 },					"Store Task Register" },
		{ "0F+00+m2", "lldt", { r_m16 },				"Load Local Descriptor Table Register" },
		{ "0F+00+m3", "ltr", { r_m16 },					"Load Task Register" },
		{ "0F+00+m4", "verr", { r_m16 },				"Verify a Segment for Reading" },
		{ "0F+00+m5", "verw", { r_m16 },				"Verify a Segment for Writing" },
		{ "0F+01+C1", "vmcall", {  },					"Call to VM Monitor" },
		{ "0F+01+C2", "vmlaunch", {  },					"Launch Virtual Machine" },
		{ "0F+01+C3", "vmresume", {  },					"Resume Virtual Machine" },
		{ "0F+01+C4", "vmxoff", {  },					"Leave VMX Operation" },
		{ "0F+01+C8", "monitor", {  },					"Set Up Monitor Address" },
		{ "0F+01+C9", "mwait", {  },					"Monitor Wait" },
		{ "0F+01+CA", "clac", {  },						"Clear AC flag in EFLAGS register" },
		{ "0F+01+m0", "sgdt", { r_m16_32 },				"Store Global Descriptor Table Register" },
		{ "0F+01+m1", "sidt", { r_m16_32 },				"Store Interrupt Descriptor Table Register" },
		{ "0F+01+m2", "lgdt", { r_m16_32 },				"Load Global Descriptor Table Register" },
		{ "0F+01+m3", "lidt", { r_m16_32 },				"Load Interrupt Descriptor Table Register" },
		{ "0F+01+m4", "smsw", { r_m16_32 },				"Store Machine Status Word" },
		{ "0F+01+m5", "smsw", { r_m16_32 },				"Store Machine Status Word" },
		{ "0F+01+m6", "lmsw", { r_m16_32 },				"Load Machine Status Word" },
		{ "0F+01+m7", "invplg", { r_m16_32 },			"Invalidate TLB Entry" },

		// To-do....Add the rest of the Two-Byte opcodes and we're done
		// Personal note: Fix 3-byte-extended opcodes..
		{ "F3+0F+10", "movss", { xmm, xmm_m32 },		"Move Scalar Single-FP Values" },
		{ "66+0F+10", "movupd", { xmm, xmm_m128 },		"Move Unaligned Packed Double-FP Value" },
		{ "F2+0F+10", "movsd", { xmm, xmm_m64 },		"Move Scalar Double-FP Value" },
		{ "0F+10", "movups", { xmm, xmm_m128 },			"Move Unaligned Packed Single-FP Values" },

		{ "10", "adc", { r_m8, r8 },					"Add with Carry" },
		{ "11", "adc", { r_m16_32, r16_32 },			"Add with Carry" },
		{ "12", "adc", { r8, r_m8 },					"Add with Carry" },
		{ "13", "adc", { r16_32, r_m16_32 },			"Add with Carry" },
		{ "14", "adc", { AL, imm8 },					"Add with Carry" },
		{ "15", "adc", { EAX, imm16_32 },				"Add with Carry" },
		{ "16", "push", { SS },							"Push Stack Segment onto the stack" },
		{ "17", "pop", { SS },							"Pop Stack Segment off of the stack" },
		{ "18", "sbb", { r_m8, r8 },					"Integer Subtraction with Borrow" },
		{ "19", "sbb", { r_m16_32, r16_32 },			"Integer Subtraction with Borrow" },
		{ "1A", "sbb", { r8, r_m8 },					"Integer Subtraction with Borrow" },
		{ "1B", "sbb", { r16_32, r_m16_32 },			"Integer Subtraction with Borrow" },
		{ "1C", "sbb", { AL, imm8 },					"Integer Subtraction with Borrow" },
		{ "1D", "sbb", { EAX, imm16_32 },				"Integer Subtraction with Borrow" },
		{ "1E", "push", { DS },							"Push Data Segment onto the stack" },
		{ "1F", "pop", { DS },							"Pop Data Segment off of the stack" },
		{ "20", "and", { r_m8, r8 },					"Logical AND" },
		{ "21", "and", { r_m16_32, r16_32 },			"Logical AND" },
		{ "22", "and", { r8, r_m8 },					"Logical AND" },
		{ "23", "and", { r16_32, r_m16_32 },			"Logical AND" },
		{ "24", "and", { AL, imm8 },					"Logical AND" },
		{ "25", "and", { EAX, imm16_32 },				"Logical AND" },
		{ "27", "daa", { AL },							"Decimal Adjust AL after Addition" },
		{ "28", "sub", { r_m8, r8 },					"Subtract" },
		{ "29", "sub", { r_m16_32, r16_32 },			"Subtract" },
		{ "2A", "sub", { r8, r_m8 },					"Subtract" },
		{ "2B", "sub", { r16_32, r_m16_32 },			"Subtract" },
		{ "2C", "sub", { AL, imm8 },					"Subtract" },
		{ "2D", "sub", { EAX, imm16_32 },				"Subtract" },
		{ "2F", "das", { AL },							"Decimal Adjust AL after Subtraction" },
		{ "30", "xor", { r_m8, r8 },					"Logical Exclusive OR" },
		{ "31", "xor", { r_m16_32, r16_32 },			"Logical Exclusive OR" },
		{ "32", "xor", { r8, r_m8 },					"Logical Exclusive OR" },
		{ "33", "xor", { r16_32, r_m16_32 },			"Logical Exclusive OR" },
		{ "34", "xor", { AL, imm8 },					"Logical Exclusive OR" },
		{ "35", "xor", { EAX, imm16_32 },				"Logical Exclusive OR" },
		{ "37", "aaa", { AL, AH },						"ASCII Adjust After Addition" },
		{ "38", "cmp", { r_m8, r8 },					"Compare Two Operands" },
		{ "39", "cmp", { r_m16_32, r16_32 },			"Compare Two Operands" },
		{ "3A", "cmp", { r8, r_m8 },					"Compare Two Operands" },
		{ "3B", "sbb", { r16_32, r_m16_32 },			"Compare Two Operands" },
		{ "3C", "sbb", { AL, imm8 },					"Compare Two Operands" },
		{ "3D", "sbb", { EAX, imm16_32 },				"Compare Two Operands" },
		{ "3F", "aas", { AL, AH },						"ASCII Adjust AL After Subtraction" },
		{ "40+r", "inc", { r16_32 },					"Increment by 1" },
		{ "48+r", "dec", { r16_32 },					"Decrement by 1" },
		{ "50+r", "push", { r16_32 },					"Push Word, Doubleword or Quadword Onto the Stack" },
		{ "58+r", "pop", { r16_32 },					"Pop a Value from the Stack" },
		{ "60", "pushad", {  },							"Push All General-Purpose Registers" },
		{ "61", "popad", {  },							"Pop All General-Purpose Registers" },
		{ "62", "bound", { r16_32, m16_32_and_16_32 },	"Check Array Index Against Bounds" },
		{ "63", "arpl", { r_m16, r16 },					"Adjust RPL Field of Segment Selector" },
		{ "68", "push", { imm16_32 },					"Push Word, Doubleword or Quadword Onto the Stack" },
		{ "69", "imul", { r16_32, r_m16_32, imm16_32 },	"Signed Multiply" },
		{ "6A", "push", { imm8 },						"Push Word, Doubleword or Quadword Onto the Stack" },
		{ "6B", "imul", { r16_32, r_m16_32, imm8 },		"Signed Multiply" },
		{ "6C", "insb", {  },							"Input from Port to String" },
		{ "6D", "insd", {  },							"Input from Port to String" },
		{ "6E", "outsb", {  },							"Output String to Port" },
		{ "6F", "outsd", {  },							"Output String to Port" },
		{ "70", "jo", { rel8 },							"Jump short if overflow (OF=1)" },
		{ "71", "jno", { rel8 },						"Jump short if not overflow (OF=0))" },
		{ "72", "jb", { rel8 },							"Jump short if below/not above or equal/carry (CF=1)" },
		{ "73", "jae", { rel8 },						"Jump short if not below/above or equal/not carry (CF=0))" },
		{ "74", "je", { rel8 },							"Jump short if zero/equal (ZF=1)" },
		{ "75", "jne", { rel8 },						"Jump short if not zero/not equal (ZF=0)" },
		{ "76", "jna", { rel8 },						"Jump short if below or equal/not above (CF=1 OR ZF=1)" },
		{ "77", "ja", { rel8 },							"Jump short if not below or equal/above (CF=0 AND ZF=0)" },
		{ "78", "js", { rel8 },							"Jump short if sign (SF=1)" },
		{ "79", "jns", { rel8 },						"Jump short if not sign (SF=0)" },
		{ "7A", "jp", { rel8 },							"Jump short if parity/parity even (PF=1)" },
		{ "7B", "jnp", { rel8 },						"Jump short if not parity/parity odd (PF=0)" },
		{ "7C", "jl", { rel8 },							"Jump short if less/not greater (SF!=OF)" },
		{ "7D", "jge", { rel8 },						"Jump short if not less/greater or equal (SF=OF)" },
		{ "7E", "jle", { rel8 },						"Jump short if less or equal/not greater ((ZF=1) OR (SF!=OF))" },
		{ "7F", "jg", { rel8 },							"Jump short if not less nor equal/greater ((ZF=0) AND (SF=OF))" },
		{ "80+m0", "add", { r_m8, imm8 },				"Add"},
		{ "80+m1", "or", { r_m8, imm8 },				"Logical Inclusive OR"},
		{ "80+m2", "adc", { r_m8, imm8 },				"Add with Carry"},
		{ "80+m3", "sbb", { r_m8, imm8 },				"Integer Subtraction with Borrow"},
		{ "80+m4", "and", { r_m8, imm8 },				"Logical AND"},
		{ "80+m5", "sub", { r_m8, imm8 },				"Subtract"},
		{ "80+m6", "xor", { r_m8, imm8 },				"Logical Exclusive OR"},
		{ "80+m7", "cmp", { r_m8, imm8 },				"Compare Two Operands"},
		{ "81+m0", "add", { r_m16_32, imm16_32 },		"Add"},
		{ "81+m1", "or", { r_m16_32, imm16_32 },		"Logical Inclusive OR"},
		{ "81+m2", "adc", { r_m16_32, imm16_32 },		"Add with Carry"},
		{ "81+m3", "sbb", { r_m16_32, imm16_32 },		"Integer Subtraction with Borrow"},
		{ "81+m4", "and", { r_m16_32, imm16_32 },		"Logical AND"},
		{ "81+m5", "sub", { r_m16_32, imm16_32 },		"Subtract"},
		{ "81+m6", "xor", { r_m16_32, imm16_32 },		"Logical Exclusive OR"},
		{ "81+m7", "cmp", { r_m16_32, imm16_32 },		"Compare Two Operands"},
		{ "82+m0", "add", { r_m8, imm8 },				"Add" },
		{ "82+m1", "or", { r_m8, imm8 },				"Logical Inclusive OR" },
		{ "82+m2", "adc", { r_m8, imm8 },				"Add with Carry" },
		{ "82+m3", "sbb", { r_m8, imm8 },				"Integer Subtraction with Borrow" },
		{ "82+m4", "and", { r_m8, imm8 },				"Logical AND" },
		{ "82+m5", "sub", { r_m8, imm8 },				"Subtract" },
		{ "82+m6", "xor", { r_m8, imm8 },				"Logical Exclusive OR" },
		{ "82+m7", "cmp", { r_m8, imm8 },				"Compare Two Operands" },
		{ "83+m0", "add", { r_m16_32, imm8 },			"Add" },
		{ "83+m1", "or", { r_m16_32, imm8 },			"Logical Inclusive OR" },
		{ "83+m2", "adc", { r_m16_32, imm8 },			"Add with Carry" },
		{ "83+m3", "sbb", { r_m16_32, imm8 },			"Integer Subtraction with Borrow" },
		{ "83+m4", "and", { r_m16_32, imm8 },			"Logical AND" },
		{ "83+m5", "sub", { r_m16_32, imm8 },			"Subtract" },
		{ "83+m6", "xor", { r_m16_32, imm8 },			"Logical Exclusive OR" },
		{ "83+m7", "cmp", { r_m16_32, imm8 },			"Compare Two Operands" },
		{ "84", "test", { r_m8, r8 },					"Logical Compare" },
		{ "85", "test", { r_m16_32, r16_32 },			"Logical Compare" },
		{ "86", "xchg", { r_m8, r8 },					"Exchange Register/Memory with Register" },
		{ "87", "xchg", { r_m16_32, r16_32 },			"Exchange Register/Memory with Register" },
		{ "88", "mov", { r_m8, r8 },					"Move" },
		{ "89", "mov", { r_m16_32, r16_32 },			"Move" },
		{ "8A", "mov", { r8, r_m8 },					"Move" },
		{ "8B", "mov", { r16_32, r_m16_32 },			"Move" },
		{ "8C", "mov", { m16, Sreg },					"Move" },
		{ "8D", "lea", { r16_32, m32 },					"Load Effective Address" },
		{ "8E", "mov", { Sreg, r_m16 },					"Move" },
		{ "8F", "pop", { r_m16_32 },					"Pop a Value from the Stack" },
		{ "90", "nop", {  },							"No Operation" },
		{ "90+r", "xchg", { EAX, r16_32 },				"Exchange Register/Memory with Register" },
		{ "98", "cbw", { AX, AL },						"Convert Byte to Word" },
		{ "99", "cwd", { AX, AL },						"Convert Doubleword to Quadword" },
		{ "9A", "callf", { ptr16_32 },					"Call Procedure" },
		{ "9B", "fwait", {  },							"Check pending unmasked floating-point exceptions" },
		{ "9C", "pushfd", {  },							"Push EFLAGS Register onto the Stack" },
		{ "9D", "popfd", {  },							"Pop Stack into EFLAGS Register" },
		{ "9E", "sahf", { AH },							"Store AH into Flags" },
		{ "9F", "lahf", { AH },							"Load Status Flags into AH Register" },
		{ "A0", "mov", { AL, moffs8 },					"Move" },
		{ "A1", "mov", { EAX, moffs16_32 },				"Move" },
		{ "A2", "mov", { moffs8, AL },					"Move" },
		{ "A3", "mov", { moffs16_32, EAX },				"Move" },
		{ "A4", "movsb", {  },							"Move Data from String to String" },
		{ "A5", "movsw", {  },							"Move Data from String to String" },
		{ "A6", "cmpsb", {  },							"Compare String Operands" },
		{ "A7", "cmpsw", {  },							"Compare String Operands" },
		{ "A8", "test", { AL, imm8 },					"Logical Compare" },
		{ "A9", "test", { EAX, imm16_32 },				"Logical Compare" },
		{ "AA", "stosb", {  },							"Store String" },
		{ "AB", "stosw", {  },							"Store String" },
		{ "AC", "lodsb", {  },							"Load String" },
		{ "AD", "lodsw", {  },							"Load String" },
		{ "AE", "scasb", {  },							"Scan String" },
		{ "AF", "scasw", {  },							"Scan String" },
		{ "B0+r", "mov", { r8, imm8 },					"Move" },
		{ "B8+r", "mov", { r16_32, imm16_32 },			"Move" },
		{ "C0+m0", "rol", { r_m8, imm8 },				"Rotate" },
		{ "C0+m1", "ror", { r_m8, imm8 },				"Rotate" },
		{ "C0+m2", "rcl", { r_m8, imm8 },				"Rotate" },
		{ "C0+m3", "rcr", { r_m8, imm8 },				"Rotate" },
		{ "C0+m4", "shl", { r_m8, imm8 },				"Shift" },
		{ "C0+m5", "shr", { r_m8, imm8 },				"Shift" },
		{ "C0+m6", "shl", { r_m8, imm8 },				"Shift" },
		{ "C0+m7", "shr", { r_m8, imm8 },				"Shift" },
		{ "C1+m0", "rol", { r_m16_32, imm8 },			"Rotate" },
		{ "C1+m1", "ror", { r_m16_32, imm8 },			"Rotate" },
		{ "C1+m2", "rcl", { r_m16_32, imm8 },			"Rotate" },
		{ "C1+m3", "rcr", { r_m16_32, imm8 },			"Rotate" },
		{ "C1+m4", "shl", { r_m16_32, imm8 },			"Shift" },
		{ "C1+m5", "shr", { r_m16_32, imm8 },			"Shift" },
		{ "C1+m6", "shl", { r_m16_32, imm8 },			"Shift" },
		{ "C1+m7", "shr", { r_m16_32, imm8 },			"Shift" },
		{ "C2", "ret", { imm16 },						"Return from procedure" },
		{ "C3", "retn", {  },							"Return from procedure" },
		{ "C4", "les", { ES, r16_32, m16_32_and_16_32 },"Load Far Pointer" },
		{ "C5", "lds", { DS, r16_32, m16_32_and_16_32 },"Load Far Pointer" },
		{ "C6", "mov", { r_m8, imm8 },					"Move" },
		{ "C7", "mov", { r_m16_32, imm16_32 },			"Move" },
		{ "C8", "enter", { EBP, imm16, imm8 },			"Make Stack Frame for Procedure Parameters" },
		{ "C9", "leave", { EBP },						"High Level Procedure Exit" },
		{ "CA", "retf", { imm16 },						"Return from procedure" },
		{ "CB", "retf", {  },							"Return from procedure" },
		{ "CC", "int 3", {  },							"Call to Interrupt Procedure" },
		{ "CD", "int", { imm8 },						"Call to Interrupt Procedure" },
		{ "CE", "into", {  },							"Call to Interrupt Procedure" },
		{ "CF", "iretd", {  },							"Interrupt Return" },
		{ "D0+m0", "rol", { r_m8, one },				"Rotate" },
		{ "D0+m1", "ror", { r_m8, one },				"Rotate" },
		{ "D0+m2", "rcl", { r_m8, one },				"Rotate" },
		{ "D0+m3", "rcr", { r_m8, one },				"Rotate" },
		{ "D0+m4", "shl", { r_m8, one },				"Shift" },
		{ "D0+m5", "shr", { r_m8, one },				"Shift" },
		{ "D0+m6", "shl", { r_m8, one },				"Shift" },
		{ "D0+m7", "shr", { r_m8, one },				"Shift" },
		{ "D1+m0", "rol", { r_m16_32, one },			"Rotate" },
		{ "D1+m1", "ror", { r_m16_32, one },			"Rotate" },
		{ "D1+m2", "rcl", { r_m16_32, one },			"Rotate" },
		{ "D1+m3", "rcr", { r_m16_32, one },			"Rotate" },
		{ "D1+m4", "shl", { r_m16_32, one },			"Shift" },
		{ "D1+m5", "shr", { r_m16_32, one },			"Shift" },
		{ "D1+m6", "shl", { r_m16_32, one },			"Shift" },
		{ "D1+m7", "shr", { r_m16_32, one },			"Shift" },
		{ "D2+m0", "rol", { r_m8, CL },					"Rotate" },
		{ "D2+m1", "ror", { r_m8, CL },					"Rotate" },
		{ "D2+m2", "rcl", { r_m8, CL },					"Rotate" },
		{ "D2+m3", "rcr", { r_m8, CL },					"Rotate" },
		{ "D2+m4", "shl", { r_m8, CL },					"Shift" },
		{ "D2+m5", "shr", { r_m8, CL },					"Shift" },
		{ "D2+m6", "shl", { r_m8, CL },					"Shift" },
		{ "D2+m7", "shr", { r_m8, CL },					"Shift" },
		{ "D3+m0", "rol", { r_m16_32, CL },				"Rotate" },
		{ "D3+m1", "ror", { r_m16_32, CL },				"Rotate" },
		{ "D3+m2", "rcl", { r_m16_32, CL },				"Rotate" },
		{ "D3+m3", "rcr", { r_m16_32, CL },				"Rotate" },
		{ "D3+m4", "shl", { r_m16_32, CL },				"Shift" },
		{ "D3+m5", "shr", { r_m16_32, CL },				"Shift" },
		{ "D3+m6", "shl", { r_m16_32, CL },				"Shift" },
		{ "D3+m7", "shr", { r_m16_32, CL },				"Shift" },
		{ "D4", "aam", { AL, AH, imm8 },				"ASCII Adjust AX After Multiply" },
		{ "D5", "aad", { AL, AH, imm8 },				"ASCII Adjust AX Before Division" },
		{ "D6", "setalc", { AL },						"Set AL If Carry" },
		{ "D7", "xlatb", { AL },						"Table Look-up Translation" },
		{ "D8+m8", "fadd", { ST, STi },					"Add" },
		{ "D8+m9", "fmul", { ST, STi },					"Multiply" },
		{ "D8+mA", "fcom", { ST, STi },					"Compare Real" },
		{ "D8+mB", "fcomp", { ST, STi },				"Compare Real and Pop" },
		{ "D8+mC", "fsub", { ST, STi },					"Subtract" },
		{ "D8+mD", "fsubr", { ST, STi },				"Reverse Subtract" },
		{ "D8+mE", "fdiv", { ST, STi },					"Divide" },
		{ "D8+mF", "fdivr", { ST, STi },				"Reverse Divide" },
		{ "D8+m0", "fadd", { STi },						"Add" },
		{ "D8+m1", "fmul", { STi },						"Multiply" },
		{ "D8+m2", "fcom", { STi },						"Compare Real" },
		{ "D8+m3", "fcomp", { STi },					"Compare Real and Pop" },
		{ "D8+m4", "fsub", { STi },						"Subtract" },
		{ "D8+m5", "fsubr", { STi },					"Reverse Subtract" },
		{ "D8+m6", "fdiv", { STi },						"Divide" },
		{ "D8+m7", "fdivr", { STi },					"Reverse Divide" },
		{ "D9+m0", "fld", { STi },						"Load Floating Point Value" },
		{ "D9+m1", "fxch", { STi },						"Exchange Register Contents" },
		{ "D9+m2", "fst", { STi },						"Store Floating Point Value" },
		{ "D9+m3", "fstp", { STi },						"Store Floating Point Value and Pop" },
		{ "D9+m4", "fldenv", { STi },					"Load x87 FPU Environment" },
		{ "D9+m5", "fldcw", { STi },					"Load x87 FPU Control Word" },
		{ "D9+m6", "fnstenv", { STi },					"Store x87 FPU Environment" },
		{ "D9+m7", "fnstcw", { STi },					"Store x87 FPU Control Word" },
		{ "DA+m8", "fcmovb", { ST, STi },				"FP Conditional Move - below (CF=1)" },
		{ "DA+m9", "fcmove", { ST, STi },				"FP Conditional Move - equal (ZF=1)" },
		{ "DA+mA", "fcmovbe", { ST, STi },				"FP Conditional Move - below or equal (CF=1 or ZF=1)" },
		{ "DA+mB", "fcmovu", { ST, STi },				"FP Conditional Move - unordered (PF=1)" },
		{ "DA+mC", "fisub", { ST, STi },				"Subtract" },
		{ "DA+mD", "fisubr", { ST, STi },				"Reverse Subtract" },
		{ "DA+mE", "fidiv", { ST, STi },				"Divide" },
		{ "DA+mF", "fidivr", { ST, STi },				"Reverse Divide" },
		{ "DA+m0", "fiadd", { STi },					"Add" },
		{ "DA+m1", "fimul", { STi },					"Multiply" },
		{ "DA+m2", "ficom", { STi },					"Compare Real" },
		{ "DA+m3", "ficomp", { STi },					"Compare Real and Pop" },
		{ "DA+m4", "fisub", { STi },					"Subtract" },
		{ "DA+m5", "fisubr", { STi },					"Reverse Subtract" },
		{ "DA+m6", "fidiv", { STi },					"Divide" },
		{ "DA+m7", "fidivr", { STi },					"Reverse Divide" },
		{ "DB+m8", "fcmovnb", { ST, STi },				"FP Conditional Move - not below (CF=0)" },
		{ "DB+m9", "fcmovne", { ST, STi },				"FP Conditional Move - not equal (ZF=0)" },
		{ "DB+mA", "fcmovnbe", { ST, STi },				"FP Conditional Move - below or equal (CF=0 and ZF=0)" },
		{ "DB+mB", "fcmovnu", { ST, STi },				"FP Conditional Move - not unordered (PF=0)" },
		{ "DB+m0", "fild", { STi },						"Load Integer" },
		{ "DB+m1", "fisttp", { STi },					"Store Integer with Truncation and Pop" },
		{ "DB+m2", "fist", { STi },						"Store Integer" },
		{ "DB+m3", "fistp", { STi },					"Store Integer and Pop" },
		{ "DB+m4", "finit", { STi },					"Initialize Floating-Point Unit" },
		{ "DB+m5", "fucomi", { STi },					"Unordered Compare Floating Point Values and Set EFLAGS" },
		{ "DB+m6", "fcomi", { STi },					"Compare Floating Point Values and Set EFLAGS" },
		{ "DB+m7", "fstp", { STi },						"Store Floating Point Value and Pop" },
		{ "DC+m8", "fadd", { STi, ST },					"Add" },
		{ "DC+m9", "fmul", { STi, ST },					"Multiply" },
		{ "DC+mA", "fcom", { STi, ST },					"Compare Real" },
		{ "DC+mB", "fcomp", { STi, ST },				"Compare Real and Pop" },
		{ "DC+mC", "fsub", { STi, ST },					"Subtract" },
		{ "DC+mD", "fsubr", { STi, ST },				"Reverse Subtract" },
		{ "DC+mE", "fdiv", { STi, ST },					"Divide" },
		{ "DC+mF", "fdivr", { STi, ST },				"Reverse Divide" },
		{ "DC+m0", "fadd", { STi },						"Add" },
		{ "DC+m1", "fmul", { STi },						"Multiply" },
		{ "DC+m2", "fcom", { STi },						"Compare Real" },
		{ "DC+m3", "fcomp", { STi },					"Compare Real and Pop" },
		{ "DC+m4", "fsub", { STi },						"Subtract" },
		{ "DC+m5", "fsubr", { STi },					"Reverse Subtract" },
		{ "DC+m6", "fdiv", { STi },						"Divide" },
		{ "DC+m7", "fdivr", { STi },					"Reverse Divide" },
		{ "DD+m8", "ffree", { STi },					"Free Floating-Point Register" },
		{ "DD+m0", "fld", { STi },						"Load Floating Point Value" },
		{ "DD+m1", "fisttp", { STi },					"Store Integer with Truncation and Pop" },
		{ "DD+m2", "fst", { STi },						"Store Floating Point Value" },
		{ "DD+m3", "fstp", { STi },						"Store Floating Point Value and Pop" },
		{ "DD+m4", "frstor", { STi },					"Restore x87 FPU State" },
		{ "DD+m5", "fucomp", { STi },					"Unordered Compare Floating Point Values and Pop" },
		{ "DD+m6", "fnsave", { STi },					"Store x87 FPU State" },
		{ "DD+m7", "fnstsw", { STi },					"Store x87 FPU Status Word" },
		{ "DE+m8", "faddp", { ST, STi },				"Add and Pop" },
		{ "DE+m9", "fmulp", { ST, STi },				"Multiply and Pop" },
		{ "DE+mA", "ficom", { ST, STi },				"Compare Real" },
		{ "DE+mB", "ficomp", { ST, STi },				"Compare Real and Pop" },
		{ "DE+mC", "fsubrp", { ST, STi },				"Reverse Subtract and Pop" },
		{ "DE+mD", "fsubp", { ST, STi },				"Subtract and Pop" },
		{ "DE+mE", "fdivrp", { ST, STi },				"Reverse Divide and Pop" },
		{ "DE+mF", "fdivp", { ST, STi },				"Divide and Pop" },
		{ "DE+m0", "fiadd", { STi },					"Add" },
		{ "DE+m1", "fimul", { STi },					"Multiply" },
		{ "DE+m2", "ficom", { STi },					"Compare Real" },
		{ "DE+m3", "ficomp", { STi },					"Compare Real and Pop" },
		{ "DE+m4", "fisub", { STi },					"Subtract" },
		{ "DE+m5", "fisubr", { STi },					"Reverse Subtract" },
		{ "DE+m6", "fidiv", { STi },					"Divide" },
		{ "DE+m7", "fdivr", { STi },					"Reverse Divide" },
		{ "DF+m8", "ffreep", { STi },					"Free Floating-Point Register and Pop" },
		{ "DF+m9", "fisttp", { r32 },					"Store Integer with Truncation and Pop" },
		{ "DF+mA", "fist", { STi },						"Store Integer" },
		{ "DF+mB", "fistp", { STi },					"Store Integer and Pop" },
		{ "DF+mC", "fnstsw", { STi },					"Store x87 FPU Status Word" },
		{ "DF+mD", "fucomip", { ST, STi },				"Unordered Compare Floating Point Values and Set EFLAGS and Pop" },
		{ "DF+mE", "fcomip", { ST, STi },				"Compare Floating Point Values and Set EFLAGS and Pop" },
		{ "DF+mF", "fistp", { r64 },					"Store Integer and Pop" },
		{ "DF+m0", "fild", { STi },						"Load Integer" },
		{ "DF+m1", "fisttp", { STi },					"Store Integer with Truncation and Pop" },
		{ "DF+m2", "fist", { STi },						"Store Integer" },
		{ "DF+m3", "fistp", { STi },					"Store Integer and Pop" },
		{ "DF+m4", "fbld", { STi },						"Load Binary Coded Decimal" },
		{ "DF+m5", "fild", { STi },						"Load Integer" },
		{ "DF+m6", "fbstp", { STi },					"Store BCD Integer and Pop" },
		{ "DF+m7", "fistp", { STi },					"Store Integer and Pop" },
		{ "E0", "loopne", { ECX, rel8 },				"Decrement count; Jump short if count!=0 and ZF=0" },
		{ "E1", "loope", { ECX, rel8 },					"Decrement count; Jump short if count!=0 and ZF=1" },
		{ "E2", "loop", { ECX, rel8 },					"Decrement count; Jump short if count!=0" },
		{ "E3", "jecxz", { rel8 },						"Jump short if eCX register is 0" },
		{ "E4", "in", { AL, imm8 },						"Input from Port" },
		{ "E5", "in", { EAX, imm8 },					"Input from Port" },
		{ "E6", "out", { imm8, AL },					"Output to Port" },
		{ "E7", "out", { imm8, EAX },					"Output to Port" },
		{ "E8", "call", { rel16_32 },					"Call Procedure" },
		{ "E9", "jmp", { rel16_32 },					"Jump" },
		{ "EA", "jmpf", { ptr16_32 },					"Jump" },
		{ "EB", "jmp short", { rel8 },					"Jump" },
		{ "EC", "in", { AL, DX },						"Input from Port" },
		{ "ED", "in", { EAX, DX },						"Input from Port" },
		{ "EE", "out", { DX, AL },						"Output to Port" },
		{ "EF", "out", { DX, EAX },						"Output to Port" },
		{ "F1", "int 1", {  },							"Call to Interrupt Procedure" },
		{ "F4", "hlt", {  },							"Halt" },
		{ "F5", "cmc", {  },							"Complement Carry Flag" },
		{ "F6+m0", "test", { r_m8, imm8 },				"Logical Compare" },
		{ "F6+m1", "test", { r_m8, imm8 },				"Logical Compare" },
		{ "F6+m2", "not", { r_m8 },						"One's Complement Negation" },
		{ "F6+m3", "neg", { r_m8 },						"Two's Complement Negation" },
		{ "F6+m4", "mul", { AX, AL, r_m8 },				"Unsigned Multiply" },
		{ "F6+m5", "imul", { AX, AL, r_m8 },			"Signed Multiply" },
		{ "F6+m6", "div", { AX, AL, AX, r_m8 },			"Unigned Divide" },
		{ "F6+m7", "idiv", { AX, AL, AX, r_m8 },		"Signed Divide" },
		{ "F7+m0", "test", { r_m16_32, imm16_32 },		"Logical Compare" },
		{ "F7+m1", "test", { r_m16_32, imm16_32 },		"Logical Compare" },
		{ "F7+m2", "not", { r_m16_32 },					"One's Complement Negation" },
		{ "F7+m3", "neg", { r_m16_32 },					"Two's Complement Negation" },
		{ "F7+m4", "mul", { EDX, EAX, r_m16_32 },		"Unsigned Multiply" },
		{ "F7+m5", "imul", { EDX, EAX, r_m16_32 },		"Signed Multiply" },
		{ "F7+m6", "div", { EDX, EAX, r_m16_32 },		"Unigned Divide" },
		{ "F7+m7", "idiv", { EDX, EAX, r_m16_32 },		"Signed Divide" },
		{ "F8", "clc", {  },							"Clear Carry Flag" },
		{ "F9", "stc", {  },							"Set Carry Flag" },
		{ "FA", "cli", {  },							"Clear Interrupt Flag" },
		{ "FB", "sti", {  },							"Set Interrupt Flag" },
		{ "FC", "cld", {  },							"Clear Direction Flag" },
		{ "FD", "std", {  },							"Set Direction Flag" },
		{ "FE+m0", "inc", { r_m8 },						"Increment by 1" },
		{ "FE+m1", "dec", { r_m8 },						"Decrement by 1" },
		{ "FE+mE", "inc", { r_m8 },						"Increment by 1" },
		{ "FE+mF", "dec", { r_m8 },						"Decrement by 1" },
		{ "FF+m0", "inc", { r_m16_32 },					"Increment by 1" },
		{ "FF+m1", "dec", { r_m16_32 },					"Decrement by 1" },
		{ "FF+m2", "call", { r_m16_32 },				"Call Procedure" },
		{ "FF+m3", "callf", { m16_32_and_16_32 },		"Call Procedure" },
		{ "FF+m4", "jmp", { r_m16_32 },					"Jump" },
		{ "FF+m5", "jmpf", { m16_32_and_16_32 },		"Jump" },
		{ "FF+m6", "push", { r_m16_32 },				"Push Word, Doubleword or Quadword Onto the Stack" },
	};

	void* procHandle = nullptr;

	uint8_t to_byte(std::string str, int offset)
	{
		uint8_t n = 0;

		if (str[offset] == '?' && str[offset + 1] == '?')
			return n;

		for (int i = offset; i < offset + 2; i++)
		{
			uint8_t b = 0;

			if (str[i] >= 0x61)
				b = str[i] - 0x57;
			else if (str[i] >= 0x41)
				b = str[i] - 0x37;
			else if (str[i] >= 0x30)
				b = str[i] - 0x30;

			if (i == offset)
				n += (b * 16);
			else {
				n += b;
			}
		}

		return n;
	}

	std::string to_str(uint8_t b)
	{
		return "";
	}

	void Open(HANDLE handle)
	{
		procHandle = handle;
	}

	inst ReadInstruction(uintptr_t address)
	{
		auto p = inst();
		p.address = address;


		// make 1 function call either way
		if (procHandle == nullptr)
		{
			memcpy(&p.bytes, reinterpret_cast<void*>(address), sizeof(p.bytes) / sizeof(uint8_t));
		}
		else 
		{
			DWORD nothing;
			ReadProcessMemory(procHandle, reinterpret_cast<void*>(address), &p.bytes, sizeof(p.bytes) / sizeof(uint8_t), &nothing);
		}

		// load prefixes
		uint8_t* at = p.bytes;
		switch (*at)
		{
		// These segments will skip the current byte.
		// We don't bother with these in the opcode table
		case OP_SEG_CS:
			at++, p.pre_flags |= PRE_SEG_CS;
			break;
		case OP_SEG_SS:
			at++, p.pre_flags |= PRE_SEG_SS;
			break;
		case OP_SEG_DS:
			at++, p.pre_flags |= PRE_SEG_DS;
			break;
		case OP_SEG_ES:
			at++, p.pre_flags |= PRE_SEG_ES;
			break;
		case OP_SEG_FS:
			at++, p.pre_flags |= PRE_SEG_FS;
			break;
		case OP_SEG_GS:
			at++, p.pre_flags |= PRE_SEG_GS;
			break;

		// These will only affect the instruction's flags because
		// we take care of the prefixes in the opcode table
		case OP_66:
			p.pre_flags |= PRE_66;
			break;
		case OP_67:
			p.pre_flags |= PRE_67;
			break;
		case OP_LOCK:
			p.pre_flags |= PRE_LOCK;
			break;
		case OP_REPNE:
			p.pre_flags |= PRE_REPNE;
			break;
		case OP_REPE:
			p.pre_flags |= PRE_REPE;
			break;
		default: break;
		}

		// store the original
		uint8_t* prev_at = at;

		for (OP_INFO op_info : OP_TABLE)
		{
			// Reset each time we check
			at = prev_at;

			uint8_t opcode_byte = to_byte(op_info.code, 0);
			uint8_t opcode_match = (*at == opcode_byte);
			uint8_t reg_from_opcode_byte = FALSE;

			// This will check if we've included up to
			// 3 prefixes in the byte string
			for (int i = 2; i < 8; i += 3)
			{
				if (op_info.code.length() > i)
				{
					// extended byte?
					if (op_info.code[i] == '+')
					{
						// check if the opcode byte determines the register
						if (op_info.code[i + 1] == 'r')
						{
							reg_from_opcode_byte = true;

							// this simple check can simplify instructons like inc/dec/push/pop
							// in our opcode table so it can do up to 8 combinations
							// All we have to put is put `40+r` (rather than 40, 41, 42, 43,...)
							opcode_match = (*at >= opcode_byte) && (*at < opcode_byte + 8);
							break;
						}
						else if (op_info.code[i + 1] == 'm' && opcode_match)
						{
							std::string str = "0";
							str += op_info.code[i + 2];

							uint8_t n = to_byte(str, 0);
							if (n >= 0 && n < 8)
							{
								// for every +8 it switches to a different opcode out of 8
								opcode_match = (*(at + 1) % 64 / 8) == n;
							}
							else {
								n -= 8;
								opcode_match = ((*(at + 1) % 64 / 8) == n && *(at + 1) >= 0xC0);
							}
							break;
						}
						else if (opcode_match) 
						{
							// in all other cases, it's an extending byte
							at++;

							opcode_byte = to_byte(op_info.code, 3);
							opcode_match = (*at == opcode_byte);
						}
					}
				}
				else {
					break;
				}
			}
			
			// this byte matches the opcode byte
			if (opcode_match)
			{
				// move onto the next byte
				at++;

				strcat(p.data, op_info.opcode_name);
				strcat(p.data, " ");

				size_t noperands = op_info.operands.size();
				p.operands = std::vector<operand>(noperands); // allocate for the # of operands
				p.info = op_info;

				uint8_t prev = MOD_NOT_FIRST;

				for (size_t c = 0; c < noperands; c++)
				{
					// c = current operand (index)
					// append this opmode to that of the corresponding operand
					p.operands[c].opmode = op_info.operands[c];

					// Returns the imm8 offset value at `x`
					// and then increases `at` by imm8 size.
					auto get_imm8 = [&p, &c, &at](auto x, bool constant)
					{
						char s_offset[8];

						if (!constant)
						{
							p.operands[c].imm8 = *x;

							if (*x > CHAR_MAX)
							{
								sprintf(s_offset, "-%02X", (UCHAR_MAX + 1) - p.operands[c].imm8);
							}
							else {
								sprintf(s_offset, "+%02X", p.operands[c].imm8);
							}
						}
						else {
							p.operands[c].disp8 = *x;
							sprintf(s_offset, "%02X", p.operands[c].disp8);
						}

						strcat(p.data, s_offset);

						at += sizeof(uint8_t);
					};

					// Returns the imm16 offset value at `x`
					// and then increases `at` by imm16 size.
					auto get_imm16 = [&p, &c, &at](auto x, bool constant)
					{
						char s_offset[8];

						if (!constant)
						{
							p.operands[c].imm16 = *reinterpret_cast<uint16_t*>(x);

							if (*x > INT16_MAX)
							{
								sprintf(s_offset, "-%04X", (UINT16_MAX + 1) - p.operands[c].imm16);
							}
							else {
								sprintf(s_offset, "+%04X", p.operands[c].imm16);
							}
						}
						else {
							p.operands[c].disp16 = *reinterpret_cast<uint16_t*>(x);
							sprintf(s_offset, "%04X", p.operands[c].disp16);
						}

						strcat(p.data, s_offset);

						at += sizeof(uint16_t);
					};

					// Returns the imm32 offset value at `x`
					// and then increases `at` by imm32 size.
					auto get_imm32 = [&p, &c, &at](auto x, bool constant)
					{
						char s_offset[16];

						if (!constant)
						{
							p.operands[c].imm32 = *reinterpret_cast<uint32_t*>(x);

							if (p.operands[c].imm32 > INT_MAX)
							{
								sprintf(s_offset, "-%08X", (UINT_MAX + 1) - p.operands[c].imm32);
							}
							else {
								sprintf(s_offset, "+%08X", p.operands[c].imm32);
							}
						} else 
						{
							p.operands[c].disp32 = *reinterpret_cast<uint32_t*>(x);
							sprintf(s_offset, "%08X", p.operands[c].disp32);
						}

						strcat(p.data, s_offset);

						at += sizeof(uint32_t);
					};

					auto get_sib = [&get_imm8, &get_imm32, &p, &at, &c](uint8_t imm)
					{
						// get the SIB byte based on the operand's MOD byte.
						// See http://www.c-jump.com/CIS77/CPU/x86/X77_0100_sib_byte_layout.htm
						// See https://www.cs.uaf.edu/2002/fall/cs301/Encoding%20instructions.htm
						// 
						// To-do: Label the values that make up scale, index, and byte
						// I didn't label too much here so it is pretty indecent atm...
						// 

						uint8_t sib_byte = *++at; // notice we skip to the next byte for this
						uint8_t r1 = (sib_byte % 64) / 8;
						uint8_t r2 = (sib_byte % 64) % 8;

						if ((sib_byte + 32) / 32 % 2 == 0 && sib_byte % 32 < 8)
						{
							// 
							strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r2)]);
						} else 
						{
							if (r2 == 5 && *(at - 1) < 64) // we need to check the previous byte in this circumstance
							{
								strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r1)]);
							} else 
							{
								strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r2)]);
								strcat(p.data, "+"); // + SIB Base
								strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r1)]);
							}

							// SIB Scale
							if (sib_byte / 64)
							{
								p.operands[c].mul = multipliers[sib_byte / 64];
								char s_multiplier[4];
								sprintf(s_multiplier, "%i", p.operands[c].mul);
								strcat(p.data, "*");
								strcat(p.data, s_multiplier);
							}
						}

						if (imm == sizeof(uint8_t)) 
						{
							get_imm8(at + 1, false);
						} else if (imm == sizeof(uint32_t) || (imm == 0 && r2 == 5))
						{
							get_imm32(at + 1, false);
						}
					};

					// Gets the relative offset value at `x`
					// and then increases `at` by rel8 size.
					auto get_rel8 = [&p, &c, &at](auto x)
					{
						char s_offset[16];

						// get the current address of where `at` is located
						uint32_t location = p.address + (reinterpret_cast<uint32_t>(x) - reinterpret_cast<uint32_t>(p.bytes));
						// base the 8-bit relative offset on it
						p.operands[c].rel8 = (location + sizeof(uint8_t) + *reinterpret_cast<uint8_t*>(x));

						sprintf(s_offset, "%08X", p.operands[c].rel8);
						strcat(p.data, s_offset);

						at += sizeof(uint8_t);
					};

					// Gets the relative offset value at `x`
					// and then increases `at` by rel16 size.
					auto get_rel16 = [&p, &c, &at](auto x)
					{
						char s_offset[16];

						// get the current address of where `at` is located
						uint32_t location = p.address + (reinterpret_cast<uint32_t>(x) - reinterpret_cast<uint32_t>(p.bytes));
						// base the 16-bit relative offset on it
						p.operands[c].rel16 = (location + sizeof(uint16_t) + *reinterpret_cast<uint16_t*>(x));

						sprintf(s_offset, "%08X", p.operands[c].rel16);
						strcat(p.data, s_offset);

						at += sizeof(uint16_t);
					};

					// Gets the relative offset value at `x`
					// and then increases `at` by rel32 size.
					auto get_rel32 = [&p, &c, &at](auto x)
					{
						char s_offset[16];

						// get the current address of where `at` is located
						uint32_t location = p.address + (reinterpret_cast<uint32_t>(x) - reinterpret_cast<uint32_t>(p.bytes));
						// base the 32-bit relative offset on it
						p.operands[c].rel32 = (location + sizeof(uint32_t) + *reinterpret_cast<uint32_t*>(x));

						sprintf(s_offset, "%08X", p.operands[c].rel32);
						strcat(p.data, s_offset);

						at += sizeof(uint32_t);
					};

					uint8_t r = prev;

					// grab the basic register initially
					if (prev == MOD_NOT_FIRST)
					{
						r = (*at % 64) / 8;
					}

					if (reg_from_opcode_byte)
					{
						r = (*(at - 1) % 64) % 8;
					}

					switch (p.operands[c].opmode)
					{
						case OP_TYPES::one:
							p.operands[c].disp8 = p.operands[c].disp16 = p.operands[c].disp32 = 1;
							strcat(p.data, "1");
							break;
						case OP_TYPES::AL:
							strcat(p.data, "al");
							break;
						case OP_TYPES::AH:
							strcat(p.data, "ah");
							break;
						case OP_TYPES::AX:
							strcat(p.data, "ax");
							break;
						case OP_TYPES::CL:
							strcat(p.data, "cl");
							break;
						case OP_TYPES::ES:
							strcat(p.data, "es");
							break;
						case OP_TYPES::SS:
							strcat(p.data, "ss");
							break;
						case OP_TYPES::DS:
							strcat(p.data, "ds");
							break;
						case OP_TYPES::GS:
							strcat(p.data, "gs");
							break;
						case OP_TYPES::FS:
							strcat(p.data, "fs");
							break;
						case OP_TYPES::EAX:
							strcat(p.data, "eax");
							break;
						case OP_TYPES::ECX:
							strcat(p.data, "ecx");
							break;
						case OP_TYPES::EBP:
							strcat(p.data, "ebp");
							break;
						case OP_TYPES::ST:
							strcat(p.data, mnemonics::st_names[p.operands[c].append_reg(0)]);
							break;
						case OP_TYPES::Sreg:
							strcat(p.data, mnemonics::sreg_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::xmm:
							strcat(p.data, mnemonics::rxmm_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::r8:
							strcat(p.data, mnemonics::r8_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::r16:
							strcat(p.data, mnemonics::r16_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::r16_32: 
						case OP_TYPES::r32:
							strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::r64:
							strcat(p.data, mnemonics::r64_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::m8: 
						case OP_TYPES::m16: 
						case OP_TYPES::m16_32: 
						case OP_TYPES::m32:
						case OP_TYPES::m64real:
						case OP_TYPES::r_m8: 
						case OP_TYPES::r_m16: 
						case OP_TYPES::r_m16_32: 
						case OP_TYPES::r_m32: 
						case OP_TYPES::m16_32_and_16_32: 
						case OP_TYPES::xmm_m32: 
						case OP_TYPES::xmm_m64: 
						case OP_TYPES::xmm_m128:
						case OP_TYPES::STi:
						{
							// To-do: Apply markers...

							if (c == 0)
							{
								prev = r;
							}

							r = (*at % 64) % 8;

							switch (*at / 64) // determine mode from `MOD` byte
							{
							case 3:
								switch(p.operands[c].opmode)
								{
									case OP_TYPES::r_m8:
									case OP_TYPES::m8:
										strcat(p.data, mnemonics::r8_names[p.operands[c].append_reg(r)]);
										break;
									case OP_TYPES::r_m16:
									case OP_TYPES::m16:
										strcat(p.data, mnemonics::r16_names[p.operands[c].append_reg(r)]);
										break;
									case OP_TYPES::xmm_m32:
									case OP_TYPES::xmm_m64:
									case OP_TYPES::xmm_m128:
										strcat(p.data, mnemonics::rxmm_names[p.operands[c].append_reg(r)]);
										break;
									case OP_TYPES::ST:
									case OP_TYPES::STi:
										strcat(p.data, mnemonics::st_names[p.operands[c].append_reg(r)]);
										break;
									default: // Anything else is going to be 32-bit
										strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r)]);
									break;
								}
								break;
							case 0:
							{
								strcat(p.data, "[");

								switch (r)
								{
								case 4:
									get_sib(0); // Translate SIB byte (no offsets)
									break;
								case 5:
									char s_disp[16];
									sprintf(s_disp, "%08X", p.operands[c].disp32 = *reinterpret_cast<uint32_t*>(at + 1));
									strcat(p.data, s_disp);
									at += sizeof(uint32_t);
									break;
								default:
									strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r)]);
									break;
								}

								strcat(p.data, "]");
								break;
							}
							case 1:
								strcat(p.data, "[");

								if (r == 4)
									get_sib(sizeof(uint8_t)); // Translate SIB byte (with BYTE offset)
								else {
									strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r)]);
									get_imm8(at + 1, false);
								}

								strcat(p.data, "]");
								break;
							case 2:
								strcat(p.data, "[");

								if (r == 4)
									get_sib(sizeof(uint32_t)); // Translate SIB byte (with DWORD offset)
								else {
									strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r)]);
									get_imm32(at + 1, false);
								}

								strcat(p.data, "]");
								break;
							}
							at++;
							break;
						}
						case OP_TYPES::imm8:
							get_imm8(at, true); // changes to a disp32
							break;
						case OP_TYPES::imm16:
							get_imm16(at, true); // changes to a disp32
							break;
						case OP_TYPES::imm16_32: 
						case OP_TYPES::imm32:
							get_imm32(at, true); // changes to a disp32
							break;
						case OP_TYPES::moffs8:
							strcat(p.data, "[");
							get_imm32(at, true); // changes to a disp32
							strcat(p.data, "]");
							break;
						case OP_TYPES::moffs16_32:
							strcat(p.data, "[");
							get_imm32(at, true); // changes to a disp32
							strcat(p.data, "]");
							break;
						case OP_TYPES::rel8:
							get_rel8(at);
							break;
						case OP_TYPES::rel16:
							get_rel16(at);
							break;
						case OP_TYPES::rel16_32: 
						case OP_TYPES::rel32:
							get_rel32(at);
							break;
						case OP_TYPES::ptr16_32:
							get_imm32(at, true);
							strcat(p.data, ":");
							get_imm16(at, true);
						break;
					}

					// move up to the next operand
					if (c < noperands - 1 && noperands > 1)
					{
						strcat(p.data, ",");
					}
				}
				
				break;
			}
		}

		p.len = reinterpret_cast<size_t>(at) - reinterpret_cast<size_t>(p.bytes);

		if (p.len == 0)
		{
			p.len = 1;
			strcpy(p.data, "???");
		}
		
		printf("Instruction length: %i\t  %s\n", p.len, p.data);

		return p;
	}

	std::vector<EyeStep::inst> ReadInstructions(uintptr_t address, int count)
	{
		uintptr_t at = address;
		auto inst_list = std::vector<EyeStep::inst>();

		for (int c = 0; c < count; c++)
		{
			auto i = ReadInstruction(at);
			inst_list.push_back(i);
			at += i.len;
		}

		return inst_list;
	}

	std::vector<EyeStep::inst> ReadInstructionRange(uintptr_t from, uintptr_t to)
	{
		uintptr_t at = from;
		auto inst_list = std::vector<EyeStep::inst>();

		while (at < to)
		{
			auto i = ReadInstruction(at);
			inst_list.push_back(i);
			at += i.len;
		}

		return inst_list;
	}
}
