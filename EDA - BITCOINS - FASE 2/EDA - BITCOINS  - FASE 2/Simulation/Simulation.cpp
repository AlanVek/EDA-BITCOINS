#include "Simulation.h"
#include "Nodes/FULL_Node.h"
#include "Nodes/SVP_Node.h"

//Simulation constructor.
Simulation::Simulation(void) : running(true), ev(Events::NOTHING)
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
				nodes.push_back(new Full_Node(io_context, node.ip, node.port, node.index));
			else
				nodes.push_back(new SVP_Node(io_context, node.ip, node.port, node.index));

			/*Sets neighbors.*/
			for (const auto& neighbor : node.neighbors) {
				auto& ngh = gui->getNode(neighbor);
				nodes.back()->newNeighbor(ngh.index, ngh.ip, ngh.port);
			}
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
		perform();
		break;

		/*Filter (POST).*/
	case Events::FILTER:
		ev = Events::FILTER;
		//nodes[getIndex()]->NEWPOST(gui->getReceiverID(), ConnectionType::POSTFILTER,)
		break;

		/*Blocks (GET).*/
	case Events::GET_BLOCKS:
		ev = Events::GET_BLOCKS;
		//nodes[getIndex()]->NEWPOST(gui->getReceiverID(), ConnectionType::GETBLOCK,)

		break;

		/*Headers (GET).*/
	case Events::GET_HEADERS:
		ev = Events::GET_HEADERS;
		//nodes[getIndex()]->NEWPOST(gui->getReceiverID(), ConnectionType::GETHEADER,)

		break;

		/*Merkleblock (POST).*/
	case Events::MERKLEBLOCK:
		ev = Events::MERKLEBLOCK;
		//nodes[getIndex()]->NEWPOST(gui->getReceiverID(), ConnectionType::POSTMERKLE,)

		break;

		/*Block (POST).*/
	case Events::POST_BLOCK:
		ev = Events::POST_BLOCK;
		//nodes[getIndex()]->NEWPOST(gui->getReceiverID(), ConnectionType::POSTBLOCK,)

		break;

		/*Transaction (POST).*/
	case Events::TRANSACTION:
		ev = Events::TRANSACTION;
		//nodes[getIndex()]->NEWPOST(gui->getReceiverID(), ConnectionType::POSTTRANS,)
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
	/*Deletes GUI.*/
	if (gui)
		delete gui;

	/*Deletes nodes.*/
	for (auto& node : nodes) {
		if (node)
			delete node;
	}
}

/*Loops through every node and performs.*/
void Simulation::perform(void) {
	for (const auto& node : nodes)
		node->perform();
}

/*Get message's sender's index*/
const unsigned int Simulation::getIndex() {
	const unsigned int& senderID = gui->getSenderID();
	int currentIndex = -1;

	/*Gets sender's index.*/
	for (unsigned int i = 0; i < nodes.size() && currentIndex == -1; i++) {
		if (nodes[i]->getID() == senderID)
			currentIndex = i;
	}

	return currentIndex;
}