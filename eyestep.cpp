#pragma warning(disable:4996)
#include <Windows.h>
#include "eyestep.h"
#include <map>

#define SIB_NOT_FIRST 255

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

	const char* conditions[16] =
	{
		"o",
		"no",
		"b",
		"nb",
		"e",
		"ne",
		"na",
		"a",
		"s",
		"ns",
		"p",
		"np",
		"l",
		"nl",
		"le",
		"g"
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
		// . . .
		// To-do: finish two-byte instructions
		{ "0F+01+C1", "vmcall", {  },					"Call to VM Monitor" },
		{ "0F+01+C2", "vmlaunch", {  },					"Launch Virtual Machine" },
		{ "0F+01+C3", "vmresume", {  },					"Resume Virtual Machine" },
		{ "0F+01+C4", "vmxoff", {  },					"Leave VMX Operation" },
		{ "0F+01+C8", "monitor", {  },					"Set Up Monitor Address" },
		{ "0F+01+C9", "mwait", {  },					"Monitor Wait" },
		{ "F3+0F+10", "movss", { xmm, xmm_m32 },		"Move Scalar Single-FP Values" },
		{ "66+0F+10", "movupd", { xmm, xmm_m128 },		"Move Unaligned Packed Double-FP Value" },
		{ "F2+0F+10", "movsd", { xmm, xmm_m64 },		"Move Scalar Double-FP Value" },
		{ "0F+10", "movups", { xmm, xmm_m128 },			"Move Unaligned Packed Single-FP Values" },
		// . . .
		{ "10", "adc", { r_m8, r8 },					"Add with Carry" },
		{ "11", "adc", { r_m16_32, r16_32 },			"Add with Carry" },
		{ "12", "adc", { r8, r_m8 },					"Add with Carry" },
		{ "13", "adc", { r16_32, r_m16_32 },			"Add with Carry" },
		{ "14", "adc", { AL, imm8 },					"Add with Carry" },
		{ "15", "adc", { EAX, imm16_32 },				"Add with Carry" },
		{ "18", "sbb", { r_m8, r8 },					"Integer Subtraction with Borrow" },
		{ "19", "sbb", { r_m16_32, r16_32 },			"Integer Subtraction with Borrow" },
		{ "1A", "sbb", { r8, r_m8 },					"Integer Subtraction with Borrow" },
		{ "1B", "sbb", { r16_32, r_m16_32 },			"Integer Subtraction with Borrow" },
		{ "1C", "sbb", { AL, imm8 },					"Integer Subtraction with Borrow" },
		{ "1D", "sbb", { EAX, imm16_32 },				"Integer Subtraction with Borrow" },
		{ "20", "and", { r_m8, r8 },					"Logical AND" },
		{ "21", "and", { r_m16_32, r16_32 },			"Logical AND" },
		{ "22", "and", { r8, r_m8 },					"Logical AND" },
		{ "23", "and", { r16_32, r_m16_32 },			"Logical AND" },
		{ "24", "and", { AL, imm8 },					"Logical AND" },
		{ "25", "and", { EAX, imm16_32 },				"Logical AND" },
		{ "28", "sub", { r_m8, r8 },					"Subtract" },
		{ "29", "sub", { r_m16_32, r16_32 },			"Subtract" },
		{ "2A", "sub", { r8, r_m8 },					"Subtract" },
		{ "2B", "sub", { r16_32, r_m16_32 },			"Subtract" },
		{ "2C", "sub", { AL, imm8 },					"Subtract" },
		{ "2D", "sub", { EAX, imm16_32 },				"Subtract" },
		{ "30", "xor", { r_m8, r8 },					"Logical Exclusive OR" },
		{ "31", "xor", { r_m16_32, r16_32 },			"Logical Exclusive OR" },
		{ "32", "xor", { r8, r_m8 },					"Logical Exclusive OR" },
		{ "33", "xor", { r16_32, r_m16_32 },			"Logical Exclusive OR" },
		{ "34", "xor", { AL, imm8 },					"Logical Exclusive OR" },
		{ "35", "xor", { EAX, imm16_32 },				"Logical Exclusive OR" },
		{ "38", "cmp", { r_m8, r8 },					"Compare Two Operands" },
		{ "39", "cmp", { r_m16_32, r16_32 },			"Compare Two Operands" },
		{ "3A", "cmp", { r8, r_m8 },					"Compare Two Operands" },
		{ "3B", "sbb", { r16_32, r_m16_32 },			"Compare Two Operands" },
		{ "3C", "sbb", { AL, imm8 },					"Compare Two Operands" },
		{ "3D", "sbb", { EAX, imm16_32 },				"Compare Two Operands" },
		{ "40+r", "inc", { r16_32 },					"Increment by 1" },
		{ "48+r", "dec", { r16_32 },					"Decrement by 1" },
		{ "50+r", "push", { r16_32 },					"Push Word, Doubleword or Quadword Onto the Stack" },
		{ "58+r", "pop", { r16_32 },					"Pop a Value from the Stack" },
		{ "60", "pushad", {  },							"Push All General-Purpose Registers" },
		{ "61", "popad", {  },							"Pop All General-Purpose Registers" },
		{ "68", "push", { imm16_32 },					"Push Word, Doubleword or Quadword Onto the Stack" },
		{ "6A", "push", { imm8 },						"Push Word, Doubleword or Quadword Onto the Stack" },

		{ "88", "mov", { r_m8, r8 },					"Move" },
		{ "89", "mov", { r_m16_32, r16_32 },			"Move" },
		{ "8A", "mov", { r8, r_m8 },					"Move" },
		{ "8B", "mov", { r16_32, r_m16_32 },			"Move" },

		{ "E8", "call", { rel16_32 },					"Call Procedure" },
		{ "E9", "jmp", { rel16_32 },					"Jump" },
		{ "C2", "ret", { imm16 },						"Return from procedure" },
		{ "C3", "retn", {  },							"Return from procedure" },
		{ "C4", "LES", { ES, r16_32, m16_32_AND_16_32 },"Load Far Pointer" },
		{ "C5", "LDS", { DS, r16_32, m16_32_AND_16_32 },"Load Far Pointer" },
	};

	void* procHandle = nullptr;

	uint8_t to_byte(std::string str, int offset = 0)
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

		uint8_t* at = p.bytes;

		switch (*at)
		{
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

		// store the original prior to checking
		uint8_t* prev_at = at;

		for (OP_INFO op_info : OP_TABLE)
		{
			// reset each time we check
			at = prev_at;

			uint8_t opcode_byte = to_byte(op_info.code, 0);
			uint8_t opcode_match = (*at == opcode_byte);

			#pragma region

			// check: is there more than just the byte in the string?
			if (op_info.code.length() > 2)
			{
				if (op_info.code[2] == '+') // `+`
				{
					// If it's a `+` then we have 2 options
					if (op_info.code[3] == 'r') // first option is `r`
					{
						// this is a simple check to simplify instructons like inc/dec/push/pop
						// in the OPCODE table so it can do up to 8 combinations
						// and all we have to put is (for example) `40+r`
						opcode_match = (*at >= opcode_byte) && (*at < opcode_byte + 8);
					}
					else
					{
						// in all other cases, it's an extending opcode byte
						if (opcode_match)
						{
							// skip this byte
							at++;

							opcode_byte = to_byte(op_info.code, 3);
							opcode_match = (*at == opcode_byte);
							
							// check also if this extended opcode is simplified
							if (op_info.code.length() > 5) // `F3+0F+`
							{
								if (op_info.code[5] == '+') // `+`
								{
									if (op_info.code[6] == 'r') // `r`
									{
										opcode_match = (*at >= opcode_byte) && (*at < opcode_byte + 8);
									}
									else {
										// We need to repeat this one more time,
										// in case of a second extending byte
										// (definitely a LOCK/REPE/REPNE prefix)
										// 
										if (opcode_match)
										{
											// skip this byte
											at++;

											opcode_byte = to_byte(op_info.code, 6); // `10`
											opcode_match = (*at == opcode_byte);
											
											// check also if this extended opcode is simplified
											if (op_info.code.length() > 8) // `F3+0F+10+`
											{
												if (op_info.code[8] == '+') // `+`
												{
													if (op_info.code[9] == 'r') // `r`
													{
														opcode_match = (*at >= opcode_byte) && (*at < opcode_byte + 8);
													}
													else {
														opcode_match = (*at == opcode_byte);
														// We can stop here
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			#pragma endregion Check for Extended Opcodes

			// this byte matches the opcode byte
			if (opcode_match)
			{
				// move onto the next byte
				at++;

				strcat(p.data, op_info.opcode_name);
				strcat(p.data, " ");

				size_t noperands = op_info.operands.size();
				p.operands = std::vector<operand>(noperands); // allocate for the # of operands

				uint8_t prev = -1;

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
							sprintf(s_offset, "%02X", p.operands[c].imm8);
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
							p.operands[c].disp16 = *x;
							sprintf(s_offset, "%04X", p.operands[c].imm16);
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

					// 'auto' makes the most sense here
					auto get_sib = [&get_imm8, &get_imm32, &p, &at, &c](uint8_t imm)
					{
						// get the SIB byte based on the operand's MOD byte
						// http://www.c-jump.com/CIS77/CPU/x86/X77_0100_sib_byte_layout.htm
						uint8_t sib_byte = *++at; // notice we skip to the next byte for this
						uint8_t r1 = (sib_byte % 64) / 8;
						uint8_t r2 = (sib_byte % 64) % 8;

						if ((sib_byte + 32) / 32 % 2 == 0 && sib_byte % 32 < 8)
						{
							strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r2)]);
						} else 
						{
							if (r2 == 5 && *(at - 1) < 64) // we need to check the previous byte in this circumstance
							{
								strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r1)]);
							} else 
							{
								strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r2)]);
								strcat(p.data, "+");
								strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r1)]);
							}

							if (sib_byte / 64)
							{
								char s_multiplier[4];
								sprintf(s_multiplier, "%i", multipliers[sib_byte / 64]);
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
					if (r == SIB_NOT_FIRST)
					{
						if (noperands == 1)
						{
							// 1-byte opcodes almost always have the register
							// stored like this
							r = (*(at - 1) % 64) % 8;
						}
						else {
							// In nearly all other cases,
							// this is how the first register is stored
							r = (*at % 64) / 8;
						}
					}

					switch (p.operands[c].opmode)
					{
						case OP_TYPES::AL:
							strcat(p.data, "al");
							break;
						case OP_TYPES::AH:
							strcat(p.data, "ah");
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
						case OP_TYPES::xmm:
							strcat(p.data, mnemonics::rxmm_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::r8:
							strcat(p.data, mnemonics::r8_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::r16_32: case OP_TYPES::r32:
							strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r)]);
							break;
						case OP_TYPES::r_m8: case OP_TYPES::r_m16_32: case OP_TYPES::r_m32: case OP_TYPES::xmm_m32: case OP_TYPES::xmm_m64: case OP_TYPES::xmm_m128:
						{
							if (c == 0) prev = r;

							r = (*at % 64) % 8;
							switch (*at / 64) // determine mode from `MOD` byte
							{
							case 3:
								if (p.operands[c].opmode == OP_TYPES::r_m8) 
								{
									strcat(p.data, mnemonics::r8_names[p.operands[c].append_reg(r)]);
								}
								else if (
									p.operands[c].opmode == OP_TYPES::xmm_m32
								 || p.operands[c].opmode == OP_TYPES::xmm_m64
								 || p.operands[c].opmode == OP_TYPES::xmm_m128
								) {
									strcat(p.data, mnemonics::rxmm_names[p.operands[c].append_reg(r)]);
								}
								else // Anything else is going to be 32-bit
								{
									strcat(p.data, mnemonics::r32_names[p.operands[c].append_reg(r)]);
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
							get_imm8(at, false); // should this even change to disp8?
							break;
						case OP_TYPES::imm16:
							get_imm16(at, false); // Ayyyyy
							break;
						case OP_TYPES::imm16_32: case OP_TYPES::imm32:
							get_imm32(at, true); // change to a disp32...
							break;
						case OP_TYPES::rel8:
							get_rel8(at);
							break;
						case OP_TYPES::rel16:
							get_rel16(at);
							break;
						case OP_TYPES::rel16_32: case OP_TYPES::rel32:
							get_rel32(at);
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
