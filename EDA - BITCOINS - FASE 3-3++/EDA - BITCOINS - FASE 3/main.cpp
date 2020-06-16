//#include <iostream>
#include "Simulation/Simulation.h"

int main()
{
	int result = -1;
	try {
		/*Creates simulation and event.*/
		Events ev;
		Simulation mySim;

		/*Main screen for node creation.*/
		mySim.mainScreen();

		/*While user hasn't asked to leave...*/
		while (mySim.isRunning()) {
			/*Generates event.*/
			ev = mySim.eventGenerator();

			/*Dispatches event.*/
			mySim.dispatch(ev);
		}

		result = 0;
	}

	/*Exception handler.*/
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return result;
}