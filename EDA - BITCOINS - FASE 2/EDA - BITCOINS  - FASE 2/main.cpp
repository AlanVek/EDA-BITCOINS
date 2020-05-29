#include <iostream>
#include "Simulation/Simulation.h"
int main()
{
	int result = -1;
	try {
		Events ev;
		Simulation mySim;

		mySim.mainScreen();

		while (mySim.isRunning()) {
			ev = mySim.eventGenerator();

			mySim.dispatch(ev);
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return result;
}