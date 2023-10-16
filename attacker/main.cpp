#include "rwptm.h"

int main()
{
	while (!rwptm::init("TotallyAccurateBattlegrounds.exe", "attacker.exe"))
	{
		std::cout << "Target not found, retrying in 1s...\n";
		Sleep(1000);
	}
	
	while (true)
	{
		std::cout << std::hex << rwptm::read_virtual_memory<short>(rwptm::target_base) << std::endl;
		Sleep(1);
	}

	system("pause");
	return 1;
}