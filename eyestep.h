#ifndef H_EYESTEP
#define H_EYESTEP
#define MAX_INSTR_READBITS 128
#pragma warning(disable:4996)

/* Made by static, 8/22/2020
 * If you modify, redistribute, or copy parts of this source code
 * I'd highly appreciate some credit.
 * Do me a favor and leave this comment in.
 * I'm too poor to afford a legal copyright so enjoy
*/

#include <cstdint>
#include <vector>
#include <string>


#define PRE_REPNE   			0x0001
#define PRE_REPE   				0x0002
#define PRE_66   				0x0004
#define PRE_67   				0x0008
#define PRE_LOCK 				0x0010
#define PRE_SEG_CS  			0x0020
#define PRE_SEG_SS  			0x0040
#define PRE_SEG_DS  			0x0080
#define PRE_SEG_ES  			0x0100
#define PRE_SEG_FS  			0x0200
#define PRE_SEG_GS  			0x0400

#define OP_LOCK					0xF0
#define OP_REPNE				0xF2
#define OP_REPE					0xF3
#define OP_66					0x66
#define OP_67					0x67
#define OP_SEG_CS				0x2E
#define OP_SEG_SS				0x36
#define OP_SEG_DS				0x3E
#define OP_SEG_ES				0x26
#define OP_SEG_FS				0x64
#define OP_SEG_GS				0x65

namespace EyeStep
{
	// See http://ref.x86asm.net/coder32.html documentation
	// These are all the operand-types currently supported
	// on the x86 chip set
	enum OP_TYPES : uint8_t
	{
		AL,
		AH,
		EAX,
		ECX,
		EDX,
		ESP,
		EBP,
		CL,
		CX,
		DX,
		Sreg,
		Ptr16_32,
		Flags,
		EFlags,
		ES,
		CS,
		DS,
		SS,
		FS,
		GS,
		r8,
		r16,
		r16_32,
		r32,
		r_m8,
		r_m16,
		r_m16_32,
		r_m16_M32,
		r_m32,
		moffs8,
		moffs16_32,
		m16_32_AND_16_32,
		m,
		m8,
		m14_28,
		m16,
		m16_32,
		m16_int,
		m32,
		m32_int,
		m32real,
		m64,
		m64real,
		m80real,
		m80dec,
		m94_108,
		m128,
		m512,
		rel8,
		rel16,
		rel16_32,
		rel32,
		imm8,
		imm16,
		imm16_32,
		imm32,
		mm,
		mm_m64,
		xmm,
		xmm0,
		xmm_m32,
		xmm_m64,
		xmm_m128,
		STi,
		ST1,
		ST,
		LDTR,
		GDTR,
		IDTR,
		TR,
		XCR,
		MSR,
		MSW,
		CRn,
		DRn,
		CR0,
		DR0,
		DR1,
		DR2,
		DR3,
		DR4,
		DR5,
		DR6,
		DR7,
		IA32_TIMESTAMP_COUNTER,
		IA32_SYS,
		IA32_BIOS,
	};

	struct OP_INFO
	{
		std::string code;
		const char* opcode_name;
		std::vector<OP_TYPES> operands;
		const char* description;
	};

	struct operand
	{
		operand()
		{
		}

		~operand()
		{
		}

		uint8_t opmode;
		std::vector<uint8_t> reg;
		uint8_t mul; // single multiplier

		uint8_t append_reg(uint8_t reg_type)
		{
			reg.push_back(reg_type);
			return reg_type;
		}

		union
		{
			uint8_t rel8;
			uint16_t rel16;
			uint32_t rel32;
		};
		
		union
		{
			uint8_t imm8;
			uint16_t imm16;
			uint32_t imm32;
		};

		union
		{
			uint8_t disp8;
			uint16_t disp16;
			uint32_t disp32;
		};
	};

	struct inst
	{
		char data[256];

		uint8_t bytes[MAX_INSTR_READBITS / 8];
		size_t len;
		uint16_t pre_flags;
		uintptr_t address;
		std::vector<operand>operands;

		inst()
		{
			data[0] = '\0';
			strcpy(data, "");

			address = NULL;
			pre_flags = NULL;
			len = NULL;
		}

		~inst() 
		{
			operands.clear();
		}

		operand source() 
		{ 
			if (operands.size() <= 0) return operand();
			return operands[0];
		}

		operand destination() 
		{ 
			if (operands.size() <= 1) return operand();
			return operands[1]; 
		}
	};

	extern void* procHandle;

	extern void Open(void* procHandle);
	extern inst ReadInstruction(uintptr_t address);
	extern std::vector<EyeStep::inst> ReadInstructions(uintptr_t address, int count);
	extern std::vector<EyeStep::inst> ReadInstructionRange(uintptr_t address_from, uintptr_t address_to);

	extern uint8_t to_byte(std::string);
	extern std::string to_str(uint8_t);
}

#endif

