#include "Simulation.h"
#include <iostream>
#include "Nodes/FULL_Node.h"
#include "Nodes/SVP_Node.h"

//Simulation constructor.
Simulation::Simulation(void) : running(true)
{
	/*Attempts to create new GUI variable.*/
	gui = new GUI;
}

/*Calls main GUI screen for node selection.*/
void Simulation::mainScreen() {
	running = gui->nodeSelectionScreen();

	/*If main screen exited successfully...*/
	if (running) {
		/*Loops through every created node.*/
		for (const auto& node : gui->getNodes()) {
			/*Creates new node.*/
			if (node.type == NodeTypes::NEW_FULL)
				nodes.push_back(new Full_Node(io_context, node.ip, node.port));
			else
				nodes.push_back(new SVP_Node(io_context, node.ip, node.port));

			/*Sets neighbors.*/
			for (const auto& neighbor : node.neighbors)
				nodes.back()->newNeighbor(neighbor->ip, neighbor->port);
		}
	}
}

//Dispatches according to event code.
void Simulation::dispatch(const Events& code) {
	switch (code) {
		/*User asked to exit.*/
	case Events::END:
		running = false;
		break;

		/*Nothing happened.*/
	case Events::NOTHING:
		break;

		/*Filter (POST).*/
	case Events::FILTER:
		break;

		/*Blocks (GET).*/
	case Events::GET_BLOCKS:
		break;

		/*Headers (GET).*/
	case Events::GET_HEADERS:
		break;

		/*Merkleblock (POST).*/
	case Events::MERKLEBLOCK:
		break;

		/*Block (POST).*/
	case Events::POST_BLOCK:
		break;

		/*Transaction (POST).*/
	case Events::TRANSACTION:
		break;
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

	for (auto& node : nodes) {
		if (node)
			delete node;
	}
}