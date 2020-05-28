#include <iostream>
#include "Simulation/Simulation.h"
#include "Simulation/FSMFramework/FSM.h"
#include "Simulation/FSMFramework/GUIEvents.h"
int main()
{
	int result = -1;
	try {
		/*General event generator.*/
		mainEventGenerator Events;

		FSM fsm;

		/*GUI event generator.*/
		GUIEvents guiEv;

		genericEvent* ev;

		/*Attach GUI events to general event generator.*/
		Events.attach(&guiEv);

		bool running = true;

		/*While user hasn't asked to exit...*/
		while (running) {
			/*Gets next event.*/
			ev = Events.getNextEvent();
			if (ev) {
				if (ev->getType() == Events::END)
					running = false;
				else {
					/*Fucking(something).*/

					fsm.cycle(ev);
				}

				delete ev;
			}
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return result;
}