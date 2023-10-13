#include "rwptm.h"

int main()
{
	rwptm::init("cs2.exe", "attacker.exe");

	while (true)
	{
		std::cout << std::hex << rwptm::read_virtual_memory<short>(rwptm::target_base) << std::endl;
		Sleep(500);
	}

	system("pause");
	return 1;
}