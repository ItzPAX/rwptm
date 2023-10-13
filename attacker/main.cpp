#include "rwptm.h"

int main()
{
	uintptr_t target_base = rwptm::attach("target.exe");
	rwptm::populate_cached_pml4();

	uintptr_t attacker_base = rwptm::attach("attacker.exe");
	rwptm::setup_pml4_table();

	std::cout << rwptm::read_virtual_memory<int>(0x5322ff7b0) << std::endl;
	rwptm::write_virtual_memory<int>(0x5322ff7b0, 420);
	std::cout << rwptm::read_virtual_memory<int>(0x5322ff7b0) << std::endl;

	system("pause");
	return 1;
}