#include <iostream>
#include "Simulation/Simulation.h"
int main()
{
	int result = -1;
	try {
		//Simulation mySim;
		result = 0;
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return result;
}