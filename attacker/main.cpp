#include "rwptm.h"

int main()
{
	rwptm::init("target.exe", "attacker.exe");

	std::cout << rwptm::read_virtual_memory<int>(0x56a3def800) << std::endl;
	rwptm::write_virtual_memory<int>(0x56a3def800, 420);
	std::cout << rwptm::read_virtual_memory<int>(0x56a3def800) << std::endl;

	system("pause");
	return 1;
}