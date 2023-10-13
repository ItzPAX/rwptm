#include <iostream>
#include <Windows.h>

int main()
{
	int num = 69;
	uintptr_t numptr = (uintptr_t) & num;

	std::cout << "VA PML4E of 0x" << std::hex << numptr << " is " << (unsigned short)((numptr >> 39) & 0x1FF) << std::endl;
	system("pause");

	while (true)
	{
		std::cout << "number is: " << std::dec << num << " located at 0x" << &num << std::endl;
		Sleep(500);
	}

	return 1;
}