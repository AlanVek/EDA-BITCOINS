#include <iostream>
#include "Simulation/Simulation.h"

int main()
{
	try {
		Simulation mySim;
		Events ev;

		while (mySim.isRunning()) {
			ev = mySim.eventGenerator();

			mySim.dispatch(ev);
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}