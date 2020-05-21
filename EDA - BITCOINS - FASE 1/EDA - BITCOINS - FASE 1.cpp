#include <iostream>
#include "jsonHandler.h"

int main()
{
	try {
		jsonHandler j;
		j.newJSON("blockChain.json");
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}