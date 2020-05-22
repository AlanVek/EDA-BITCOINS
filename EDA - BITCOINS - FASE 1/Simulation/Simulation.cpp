#include "Simulation.h"
#include <iostream>
#include <functional>

using namespace std::placeholders;

//Simulation constructor.
Simulation::Simulation(void) : running(true)
{
	gui = new GUI;
}

//Polls GUI and dispatches according to button code.
void Simulation::dispatch(const Events& code) {
	switch (code) {
		/*User asked to exit.*/
	case Events::END:
		running = false;
		break;
	case Events::NEW_FILE:
		blockChain.loadBlockChain(gui->getFilename());
	case Events::BLOCKID:
		gui->setInfoShower()
	default:
		break;
	}
}

/*Generates event from GUI.*/
const Events Simulation::eventGenerator() { return gui->checkStatus(); }

/*Getter.*/
bool Simulation::isRunning(void) { return running; }

/*Simulation destructor. Deletes used resources.*/
Simulation::~Simulation() {
	if (gui)
		delete gui;
}