#include <iostream>
#include "Simulation/Simulation.h"

int main()
{
	try {
		Simulation mySim;
		Events ev;

		/*While user hasn't asked to leave...*/
		while (mySim.isRunning()) {
			/*Generates GUI event.*/
			ev = mySim.eventGenerator();

			/*Dispatches said event.*/
			mySim.dispatch(ev);
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}