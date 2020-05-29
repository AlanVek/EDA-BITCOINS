#include "Simulation.h"
#include "Nodes/FULL_Node.h"
#include "Nodes/SVP_Node.h"

//Simulation constructor.
Simulation::Simulation(void) : running(true)
{
	/*Attempts to create new GUI variable.*/
	gui = new GUI;
}

void Simulation::mainScreen() {
	running = gui->nodeSelectionScreen();

	if (running) {
		for (const auto& node : gui->getNodes()) {
			if (node.type == NodeTypes::NEW_FULL)
				nodes.push_back(new Full_Node(io_context));
			else
				nodes.push_back(new SVP_Node(io_context));

			for (const auto& neighbor : node.neighbors)
				nodes.back()->newNeighbor(neighbor->ip, neighbor->port);
		}
	}
}

//Polls GUI and dispatches according to button code.
void Simulation::dispatch(const Events& code) {
	switch (code) {
		/*User asked to exit.*/
	case Events::END:
		running = false;
		break;
	default:
		break;
	}

	//gui->actionSolved();
}

/*Generates event from GUI.*/
const Events Simulation::eventGenerator() { return gui->checkStatus(); }

/*Getter.*/
bool Simulation::isRunning(void) { return running; }

/*Simulation destructor. Deletes used resources.*/
Simulation::~Simulation() {
	if (gui)
		delete gui;

	for (auto node : nodes) {
		if (node)
			delete node;
	}
}