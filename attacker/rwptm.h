#pragma once
#include "drv.h"
#include <iostream>
#include <unordered_map>

namespace rwptm
{
	typedef struct _PML4E
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // Must be 0 for PML4E.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PML4E, * PPML4E;

	wnbios_lib wnbios;

	std::unordered_map<int, PML4E> cached_pml4;

	// key1 = original pml4e index | key2 = new pml4e index
	std::unordered_map<int, int> translation_table;

	// internally attaches to a process and returns its base address
	uintptr_t attach(const char* image_name)
	{
		return wnbios.get_process_base(image_name);
	}

	// populates internal pml4 cache you should call this when attached to the target pocess
	void populate_cached_pml4()
	{
		std::cout << "Copying PML4 of target process to local cache\n";
		if (!wnbios.attached_proc || !wnbios.cr3)
		{
			std::cout << "Not attached to any processing aborting...\n";
			return;
		}

		// traverse all 512 entries in the pml4
		for (int i = 0; i < 512; i++)
		{
			PML4E pml4e;
			if (!wnbios.read_physical_memory((wnbios.cr3 + i * sizeof(uintptr_t)), &pml4e, sizeof(PML4E)))
			{
				std::cout << "Read failed\n";
				return;
			}

			// page backs physical memory
			if (pml4e.Present)
			{
				cached_pml4[i] = pml4e;
			}
		}

		std::cout << "Stored " << cached_pml4.size() << " PML4 entries\n";
	}

	// copy all present pml4e into local pml4 and setup translation table
	void setup_pml4_table()
	{
		std::cout << "Copying PML4 of target process to local pml4\n";
		if (!wnbios.attached_proc || !wnbios.cr3)
		{
			std::cout << "Not attached to any processing aborting...\n";
			return;
		}

		// populate local pml4 with target pml4 entries
		for (auto& pml4e_map : cached_pml4)
		{
			PML4E pml4e_target = pml4e_map.second;

			for (int i = 0; i < 512; i++)
			{
				PML4E pml4e_attacker;
				if (!wnbios.read_physical_memory((wnbios.cr3 + i * sizeof(uintptr_t)), &pml4e_attacker, sizeof(PML4E)))
				{
					std::cout << "Read failed\n";
					return;
				}

				// page is free for our use
				if (!pml4e_attacker.Present)
				{
					wnbios.write_physical_memory(wnbios.cr3 + i * sizeof(uintptr_t), &pml4e_target, sizeof(PML4E));
					translation_table[pml4e_map.first] = i;
					break;
				}
			}
		}
	}

	uintptr_t correct_virtual_address(uintptr_t virtual_address)
	{
		const uint64_t originalAddress = virtual_address;
		uint64_t newAddress;

		unsigned short originalPML4 = (unsigned short)((originalAddress >> 39) & 0x1FF);

		const uint64_t pml4Mask = 0x0000FF8000000000;

		newAddress = originalAddress & ~pml4Mask;

		newAddress |= (uint64_t)(translation_table[originalPML4]) << 39;

		return newAddress;
	}

	// -------- USER FUNCS ---------
	uintptr_t target_base = 0;

	void init(const char* target_application, const char* local_application)
	{
		target_base = rwptm::attach(target_application);
		rwptm::populate_cached_pml4();

		uintptr_t attacker_base = rwptm::attach(local_application);
		rwptm::setup_pml4_table();

		// unload driver we are done here
		wnbios.unload_driver();
	}

	template <typename T>
	T read_virtual_memory(uintptr_t address)
	{
		T out;
		uintptr_t fixed_addr = correct_virtual_address(address);
		memcpy(&out, (void*)fixed_addr, sizeof(T));
		return out;
	}

	template <typename T>
	void write_virtual_memory(uintptr_t address, T val)
	{
		uintptr_t fixed_addr = correct_virtual_address(address);
		memcpy((void*)fixed_addr, (void*)&val, sizeof(T));
	}
}