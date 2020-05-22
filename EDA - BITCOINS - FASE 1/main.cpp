#include <iostream>
#include "BlockChain.h"

int main()
{
	try {
		BlockChain bc("blockChain.json");

		bc.printBlockData();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}