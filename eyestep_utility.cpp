#include <Windows.h>
#include "eyestep_utility.h"

namespace EyeStep
{
	namespace util
	{
		DWORD useless;

		DWORD setMemoryPage(uint32_t address, DWORD protect, size_t size)
		{
			DWORD oldProtect;
			VirtualProtect(reinterpret_cast<void*>(address), size, protect, &oldProtect);
			return oldProtect;
		}


		void writeByte(uint32_t address, uint8_t value)
		{
			if (!external_mode)
				*reinterpret_cast<uint8_t*>(address) = value;
			else 
			{
				uint8_t bytes[sizeof(value)];
				bytes[0] = value;
				WriteProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(value), &useless);
			}
		}

		void writeBytes(uint32_t address, void* value, size_t count)
		{
			if (!external_mode)
				memcpy(reinterpret_cast<void*>(address), value, count);
			else
				WriteProcessMemory(current_proc, reinterpret_cast<void*>(address), value, count, &useless);
		}

		void writeShort(uint32_t address, uint16_t value)
		{
			if (!external_mode)
				*reinterpret_cast<uint16_t*>(address) = value;
			else
			{
				uint8_t bytes[sizeof(value)];
				*reinterpret_cast<uint16_t*>(bytes) = value;
				WriteProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(value), &useless);
			}
		}

		void writeInt(uint32_t address, uint32_t value)
		{
			if (!external_mode)
				*reinterpret_cast<uint32_t*>(address) = value;
			else
			{
				uint8_t bytes[sizeof(value)];
				*reinterpret_cast<uint32_t*>(bytes) = value;
				WriteProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(value), &useless);
			}
		}

		void writeFloat(uint32_t address, float value)
		{
			if (!external_mode)
				*reinterpret_cast<float*>(address) = value;
			else
			{
				uint8_t bytes[sizeof(value)];
				*reinterpret_cast<float*>(bytes) = value;
				WriteProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(value), &useless);
			}
		}

		void writeQword(uint32_t address, uint64_t value)
		{
			if (!external_mode)
				*reinterpret_cast<uint64_t*>(address) = value;
			else
			{
				uint8_t bytes[sizeof(value)];
				*reinterpret_cast<uint64_t*>(bytes) = value;
				WriteProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(value), &useless);
			}
		}

		void writeDouble(uint32_t address, double value)
		{
			if (!external_mode)
				*reinterpret_cast<double*>(address) = value;
			else
			{
				uint8_t bytes[sizeof(value)];
				*reinterpret_cast<double*>(bytes) = value;
				WriteProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(value), &useless);
			}
		}


		uint8_t readByte(uint32_t address)
		{
			if (!external_mode)
				return *reinterpret_cast<uint8_t*>(address);

			uint8_t bytes[sizeof(uint8_t)];
			ReadProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(bytes), &useless);

			return bytes[0];
		}

		uint8_t* readBytes(uint32_t address, size_t count)
		{
			uint8_t* bytes = new uint8_t[count];

			if (!external_mode)
			{
				memcpy(bytes, reinterpret_cast<void*>(address), count);
				return bytes;
			}

			ReadProcessMemory(current_proc, reinterpret_cast<void*>(address), bytes, count, &useless);
			return bytes;
		}

		uint16_t readShort(uint32_t address)
		{
			if (!external_mode)
				return *reinterpret_cast<uint16_t*>(address);

			uint8_t bytes[sizeof(uint16_t)];
			ReadProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(bytes), &useless);

			return *reinterpret_cast<uint16_t*>(bytes);
		}

		uint32_t readInt(uint32_t address)
		{
			if (!external_mode)
				return *reinterpret_cast<uint32_t*>(address);

			uint8_t bytes[sizeof(uint32_t)];
			ReadProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(bytes), &useless);

			return *reinterpret_cast<uint32_t*>(bytes);
		}

		float readFloat(uint32_t address)
		{
			if (!external_mode)
				return *reinterpret_cast<float*>(address);

			uint8_t bytes[sizeof(float)];
			ReadProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(bytes), &useless);

			return *reinterpret_cast<float*>(bytes);
		}

		uint64_t readQword(uint32_t address)
		{
			if (!external_mode)
				return *reinterpret_cast<uint64_t*>(address);

			uint8_t bytes[sizeof(uint64_t)];
			ReadProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(bytes), &useless);

			return *reinterpret_cast<uint64_t*>(bytes);
		}

		double readDouble(uint32_t address)
		{
			if (!external_mode)
				return *reinterpret_cast<double*>(address);

			uint8_t bytes[sizeof(double)];
			ReadProcessMemory(current_proc, reinterpret_cast<void*>(address), &bytes, sizeof(bytes), &useless);

			return *reinterpret_cast<double*>(bytes);
		}


		void freeBytes(uint8_t* bytes)
		{
			delete[] bytes;
		}

		uint32_t rebase(uint32_t address)
		{
			return reinterpret_cast<uint32_t>(base_module) + address;
		}

		uint32_t aslr(uint32_t address)
		{
			return (reinterpret_cast<uint32_t>(base_module) + address) - 0x400000;
		}

		uint32_t unaslr(uint32_t address)
		{
			return (address - reinterpret_cast<uint32_t>(base_module)) + 0x400000;
		}

		bool isRel(uint32_t address)
		{
			return (getRel(address) % 16 == 0);
		}

		bool isCall(uint32_t address)
		{
			return (
				isRel(address)
			 && getRel(address) > reinterpret_cast<uint32_t>(base_module)
			 && getRel(address) < reinterpret_cast<uint32_t>(base_module) + base_module_size
			);
		}

		bool isPrologue(uint32_t address)
		{
			return (
				// Ensure that it's aligned (helps to filter it immensely)
				(address % 16 == 0) 
			 &&
				// Check for 3 different prologues, each with different registers
				((readByte(address) == 0x55 && readByte(address + 1) == 0xEC8B)
			  || (readByte(address) == 0x53 && readByte(address + 1) == 0xDC8B)
			  || (readByte(address) == 0x56 && readByte(address + 1) == 0xF48B))
			);
		}

		bool isEpilogue(uint32_t address)
		{
			return (
				(readShort(address - 1) == 0xC35D)
			 ||
				(readShort(address - 1) == 0xC25D
			 &&  readShort(address + 1) >= 0
			 &&  readShort(address + 1) % 4 == 0
				)
			);
		}

		// determines whether the address is
		// part of the program's .text/code segment
		// by simply checking for 16 null bytes
		bool isValidCode(uint32_t address)
		{
			return !(readQword(address) == NULL && readQword(address + 8) == NULL);
		}

		uint32_t getRel(uint32_t address)
		{
			return (address + 5 + static_cast<signed int>(readInt(address + 1)));
		}

		uint32_t nextPrologue(uint32_t address)
		{
			uint32_t at = address;

			if (isPrologue(at))
			{
				at += 16;
			}
			else
			{
				at += (at % 16);
			}

			while (!(isPrologue(at) && isValidCode(at)))
			{
				at += 16;
			}

			return at;
		}

		uint32_t prevPrologue(uint32_t address)
		{
			uint32_t at = address;

			if (isPrologue(at))
			{
				at -= 16;
			}
			else
			{
				at -= (at % 16);
			}

			while (!(isPrologue(at) && isValidCode(at)))
			{
				at -= 16;
			}

			return at;
		}

		uint32_t getPrologue(uint32_t address)
		{
			return (isPrologue(address)) ? address : prevPrologue(address);
		}

		uint32_t nextCall(uint32_t address, bool location, bool prologue)
		{
			uint32_t at = address;

			if (readByte(at) == 0xE8 || readByte(at) == 0xE9)
			{
				at++;
			}

			while (1)
			{
				if ((
					readByte(at) == 0xE8
				 || readByte(at) == 0xE9
					)
				 &&
					isCall(at)
				){
					bool has_prologue = true;

					// check if we need to get the prologue
					if (prologue && !isPrologue(getRel(at)))
					{
						has_prologue = false;
					}

					if (has_prologue)
					{
						if (location)
						{
							return at;
						}
						else 
						{
							return getRel(at);
						}
					}
				}
				at++;
			}

			return (prologue) ? getPrologue(at) : at;
		}

		uint32_t prevCall(uint32_t address, bool location, bool prologue)
		{
			uint32_t at = address;

			if (readByte(at) == 0xE8 || readByte(at) == 0xE9)
			{
				at--;
			}

			while (1)
			{
				if ((
					readByte(at) == 0xE8
				 || readByte(at) == 0xE9
					)
				 && 
					isCall(at)
				){
					bool has_prologue = true;

					// check if we need to get the prologue
					if (prologue && !isPrologue(getRel(at)))
					{
						has_prologue = false;
					}

					if (has_prologue)
					{
						if (location)
						{
							return at;
						}
						else
						{
							return getRel(at);
						}
					}
				}
				at--;
			}

			return (prologue) ? getPrologue(at) : at;
		}

		uint32_t nextRef(uint32_t start, uint32_t func_search, bool prologue)
		{
			uint32_t at = start;

			while (1)
			{
				if ((
					readByte(at) == 0xE8
				 || readByte(at) == 0xE9
					)
				&& 
					getRel(at) == func_search
				){
					break;
				}

				at++;
			}

			return (prologue) ? getPrologue(at) : at;
		}

		uint32_t prevRef(uint32_t start, uint32_t func_search, bool prologue)
		{
			uint32_t at = start;

			while (1)
			{
				if ((
					readByte(at) == 0xE8
				 || readByte(at) == 0xE9
					)
				&& 
					getRel(at) == func_search
				){
					break;
				}

				at--;
			}

			return (prologue) ? getPrologue(at) : at;
		}

		uint32_t nextPointer(uint32_t start, uint32_t ptr_search, bool prologue)
		{
			uint32_t at = start + sizeof(uint32_t);

			while (1)
			{
				if (readInt(at) == ptr_search)
				{
					break;
				}
				at++;
			}

			return (prologue) ? getPrologue(at) : at;
		}

		uint32_t prevPointer(uint32_t start, uint32_t ptr_search, bool prologue)
		{
			uint32_t at = start;

			while (1)
			{
				if (readInt(at) == ptr_search)
				{
					break;
				}
				at--;
			}

			return (prologue) ? getPrologue(at) : at;
		}

		std::vector<uint32_t> getCalls(uint32_t address)
		{
			auto calls = std::vector<uint32_t>();

			uint32_t at = address;
			uint32_t func_end = nextPrologue(at);

			while (at < func_end)
			{
				if ((
					readByte(at) == 0xE8 
				 || readByte(at) == 0xE9
					)
				&& 
					isPrologue(getRel(at))
				){
					calls.push_back(getRel(at));
					at += 5;
					continue;
				}
				at++;
			}

			return calls;
		}

		std::vector<uint32_t> getPointers(uint32_t address)
		{
			auto pointers = std::vector<uint32_t>();

			uint32_t at = address;
			uint32_t func_end = nextPrologue(at);

			while (at < func_end)
			{
				auto i = EyeStep::read(at);

				if (i.source().flags & OP_DISP32 && i.source().disp32 % 4 == 0)
				{
					pointers.push_back(i.source().disp32);
				}
				else if (i.destination().flags & OP_DISP32 && i.destination().disp32 % 4 == 0)
				{
					pointers.push_back(i.destination().disp32);
				}

				at += i.len;
			}

			return pointers;
		}
	}



	namespace scanner
	{
		bool compare_bytes(const uint8_t* location, const uint8_t* aob, const char* mask)
		{
			for (; *mask; ++aob, ++mask, ++location)
			{
				if (*mask == '.' && *location != *aob)
				{
					return 0;
				}
			}
			return 1;
		}

		// aob			- array of byte string to scan
		// code			- true if restricting the scan to the '.text' section
		// align		- defaults to 1. set to 4 if going by 4 bytes (extremely fast string scans)
		// endresult	- result (#) to stop the scan at
		// checks		- additional checks at the offsets from the location of the aob, during the scan; 
		//				  these can narrow the results, make the scan faster, and increase accuracy
		// 
		scan_results scan(const char* aob, bool code, int align, int endresult, std::vector<scan_check>checks)
		{
            auto results = std::vector<uint32_t>();

            MEMORY_BASIC_INFORMATION mbi = { 0 };

            DWORD bytes_read;
            uint32_t protection = 0;
            uint32_t start = 0;
            uint32_t end = 0;

            uint8_t* pattern = new uint8_t[128];
            ZeroMemory(pattern, 128);

            char* mask = new char[128];
            ZeroMemory(mask, 128);

            // reinterprets the AOB string as a string mask
            for (int i = 0, j = 0; i < lstrlenA(aob); i++)
            {
                if (aob[i] == 0x20)
                {
                    continue;
                }

                char x[2];
                x[0] = aob[i];
                x[1] = aob[1 + i++];

                if (x[0] == '?' && x[1] == '?')
                {
                    pattern[j] = 0;
                    mask[j++] = '?';
                }
                else
                {
                    // convert 2 chars to byte
                    int id = 0;
                    int n = 0;

                    convert:        if (x[id] > 0x60) n = x[id] - 0x57; // n = A-F (10-16)
                    else if (x[id] > 0x40) n = x[id] - 0x37; // n = a-f (10-16)
                    else if (x[id] >= 0x30) n = x[id] - 0x30; // number chars

                    if (id != 0)
                        pattern[j] += n;
                    else
                    {
                        id++, pattern[j] += (n * 16);
                        goto convert;
                    }

                    mask[j++] = '.';
                }
            }

            if (!code)
            {
                SYSTEM_INFO siSysInfo;
                GetSystemInfo(&siSysInfo);

                // Restrict the scan to virtual memory
                start = reinterpret_cast<uint32_t>(base_module) + base_module_size;
				end = reinterpret_cast<uint32_t>(siSysInfo.lpMaximumApplicationAddress);
            }
            else
            {
                start = reinterpret_cast<uint32_t>(base_module);
                end = start;

                if (external_mode)
                {
                    uint8_t bytes[1024];
                    ReadProcessMemory(current_proc, base_module, &bytes, 1024, &bytes_read);

                    end = reinterpret_cast<uint32_t>(bytes);
                }

                // look for first `.rdata` marker
                while (*reinterpret_cast<uint32_t*>(end) != 0x6164722E) // ".rda"
                {
                    end++;
                }
                // jump to it
                end = (start + *reinterpret_cast<uint32_t*>(end + 12)) - 0x4000; 
            }

            while (start < end)
            {
                if (!external_mode)
                {
                    VirtualQuery(reinterpret_cast<void*>(start), &mbi, sizeof(mbi));
                }
                else {
                    VirtualQueryEx(current_proc, reinterpret_cast<void*>(start), &mbi, sizeof(mbi));
                }

                // Make sure the memory is committed, matches our protection, and isn't PAGE_GUARD.
                if ((mbi.State & MEM_COMMIT) && !(mbi.Protect == PAGE_NOACCESS || mbi.Protect == PAGE_NOCACHE || mbi.Protect & PAGE_GUARD))
                {
                    uint32_t region_base = reinterpret_cast<uint32_t>(mbi.BaseAddress);
					uint32_t i = region_base - (region_base % 16);

                    // Scan all the memory in the region.
                    while (i < region_base + mbi.RegionSize)
                    {
                        bool bytes_match = false;

                        if (!external_mode)
                        {
                            bytes_match = compare_bytes(reinterpret_cast<uint8_t*>(i), pattern, mask);
                        }
                        else {
                            uint8_t* bytes = new uint8_t[mbi.RegionSize];

                            if (ReadProcessMemory(current_proc, reinterpret_cast<void*>(i), bytes, mbi.RegionSize, &bytes_read) == 0)
                            {
                                bytes_match = compare_bytes(bytes, pattern, mask);
                            }

                            delete[] bytes;
                        }

                        if (bytes_match)
                        {
                            if (checks.size() == 0)
                                results.push_back(i);
                            else
                            {
                                // Go through a series of extra checks,
                                // make sure all are passed before it's a valid result
                                size_t checks_pass = 0;

                                for (scan_check check : checks)
                                {
                                    switch (check.type)
                                    {
                                    case byte_equal:
                                        if (*reinterpret_cast<uint8_t*>(i + check.offset) == reinterpret_cast<uint8_t>(check.value)) checks_pass++;
                                        break;
                                    case word_equal:
                                        if (*reinterpret_cast<uint16_t*>(i + check.offset) == reinterpret_cast<uint16_t>(check.value)) checks_pass++;
                                        break;
                                    case int_equal:
                                        if (*reinterpret_cast<uint32_t*>(i + check.offset) == reinterpret_cast<uint32_t>(check.value)) checks_pass++;
                                        break;
                                    case byte_notequal:
                                        if (*reinterpret_cast<uint8_t*>(i + check.offset) != reinterpret_cast<uint8_t>(check.value)) checks_pass++;
                                        break;
                                    case word_notequal:
                                        if (*reinterpret_cast<uint16_t*>(i + check.offset) != reinterpret_cast<uint16_t>(check.value)) checks_pass++;
                                        break;
                                    case int_notequal:
                                        if (*reinterpret_cast<uint32_t*>(i + check.offset) != reinterpret_cast<uint32_t>(check.value)) checks_pass++;
                                        break;
                                    }
                                }

                                if (checks_pass == checks.size())
                                {
                                    results.push_back(i);
                                }
                            }
                            if (endresult > 0 && results.size() >= endresult)
                            {
                                break;
                            }
                        }

						i += align;
                    }
				}
				else {
					printf("Skipping region %08X\n", mbi.BaseAddress);
				}

                // Move onto the next region of memory.
                start += mbi.RegionSize;
            }

            delete[] mask;
            delete[] pattern;

            return results;
		}

		// converts a string like "Test" to an AOB string "54 65 73 74"
		std::string aobstring(const char* str)
		{
			std::string aob = "";

			for (int i = 0; i < lstrlenA(str); i++)
			{
				auto b_char = static_cast<uint8_t>(str[i]);
				aob += to_str(b_char);

				if (i < lstrlenA(str) - 1)
				{
					aob += 0x20;
				}
			}

			return aob;
		}

		// converts a result to an AOB string
		// for example 0x110CBED0 --> "D0 BE 0C 11"
		std::string ptrstring(uint32_t ptr)
		{
			std::string aob = "";

			uint8_t* bytes = reinterpret_cast<uint8_t*>(ptr);

			aob += to_str(bytes[3]) + " ";
			aob += to_str(bytes[2]) + " ";
			aob += to_str(bytes[1]) + " ";
			aob += to_str(bytes[0]);

			return aob;
		}

		scan_results scan_xrefs(const char* str, int nresult)
		{
			scan_results result_list = scan(aobstring(str).c_str(), false, 4, nresult);
			if (result_list.size() > 0)
			{
				return scan(ptrstring(result_list.back()).c_str());
			}
			else {
				throw std::exception("No results found for string");
				return scan_results();
			}
		}

		scan_results scan_xrefs(uint32_t result)
		{
			return scan(ptrstring(result).c_str());
		}
	}
}
