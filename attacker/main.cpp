#include "rwptm.h"

int main()
{
	uintptr_t target_base = rwptm::attach("target.exe");
	rwptm::populate_cached_pml4();

	uintptr_t attacker_base = rwptm::attach("attacker.exe");
	rwptm::setup_pml4_table();

	uintptr_t corrected_va = rwptm::correct_virtual_address(0x1d683ff820);

	int new_val = 420;
	memcpy((void*)corrected_va, &new_val, sizeof(new_val));

	return 1;
}