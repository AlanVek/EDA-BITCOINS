#include "Simulation.h"
#include "Nodes/Full_Node.h"
#include "Nodes/SPV_Node.h"

//Simulation constructor.
Simulation::Simulation(void) : running(true), ev(Events::NOTHING)
{
	/*Attempts to create new GUI variable.*/
	gui = new GUI;
}

/*Calls main GUI screen for node selection.*/
void Simulation::mainScreen() {
	bool newNet;
	running = gui->nodeSelectionScreen(&newNet);

	/*If main screen exited successfully...*/
	if (running) {
		if (newNet) {
			/*CREATE NEW NETWORK.*/
		}
		else {
			newNodes();
		}
		gui->setRealNodes(nodes);
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
		nodes[getIndex()]->perform(ConnectionType::POSTFILTER, gui->getReceiverID(), (std::string) "0", NULL/*gui->getKey()*/);
		gui->infoGotten();

		break;

		/*Blocks (GET).*/
	case Events::GET_BLOCKS:
		ev = Events::GET_BLOCKS;
		nodes[getIndex()]->perform(ConnectionType::GETBLOCK, gui->getReceiverID(), "84CB2573", 1);
		gui->infoGotten();

		break;

		/*Headers (GET).*/
	case Events::GET_HEADERS:
		ev = Events::GET_HEADERS;
		nodes[getIndex()]->perform(ConnectionType::GETHEADER, gui->getReceiverID(), "84CB2573", 1);
		gui->infoGotten();

		break;

		/*Merkleblock (POST).*/
	case Events::MERKLEBLOCK:
		ev = Events::MERKLEBLOCK;
		nodes[getIndex()]->perform(ConnectionType::POSTMERKLE, gui->getReceiverID(), "84CB2573", "7B857A14"/*gui->getTransactionID()*/);
		gui->infoGotten();
		break;

		/*Block (POST).*/
	case Events::POST_BLOCK:
		ev = Events::POST_BLOCK;
		nodes[getIndex()]->perform(ConnectionType::POSTBLOCK, gui->getReceiverID(),/* gui->getBlockID()*/"84CB2573", NULL);
		gui->infoGotten();
		break;

		/*Transaction (POST).*/
	case Events::TRANSACTION:
		ev = Events::TRANSACTION;
		nodes[getIndex()]->perform(ConnectionType::POSTTRANS, gui->getReceiverID(), gui->getWallet(), gui->getAmount());
		gui->infoGotten();

		break;
	case Events::UPDATE:
		newNodes();
		gui->setRealNodes(nodes);
	case Events::KEEPCREATING:

		/*CONTINUE NETWORK CREATION*/

		/*IF (NETWORK DONE) GUI->NETWORKDONE();*/

		break;
	default:
		break;
	}
}

/*Generates event from GUI and polls io_context.*/
const Events Simulation::eventGenerator() {
	io_context.poll_one();

	generateMsg();

	return gui->checkStatus();
}

void Simulation::generateMsg() {
	for (const auto& node : nodes) {
		switch (node->getClientState()) {
		case ConnectionState::PERFORMING:
			gui->updateMsg("\nNode " + std::to_string(node->getID()) + " is performing a client request.");
			break;
		case ConnectionState::FINISHED:
			gui->updateMsg("\nNode " + std::to_string(node->getID()) + " finished the request.");
			gui->setRealNodes(nodes);
			break;
		default:
			break;
		}

		switch (node->getServerState()) {
			int client_reception;
		case ConnectionState::CONNECTIONOK:
			if ((client_reception = node->getClientPort()) != -1) {
				gui->updateMsg("\nNode " + std::to_string(node->getID()) + " is answering a request from node " + std::to_string(client_reception) + ". Data was OK");
			}
			break;
		case ConnectionState::CONNNECTIONFAIL:
			if ((client_reception = node->getClientPort()) != -1) {
				gui->updateMsg("\nNode " + std::to_string(node->getID()) + " is answering a request from node " + std::to_string(client_reception) + ". Data was NOT OK");
			}
			break;
		case ConnectionState::FINISHED:
			gui->updateMsg("\nNode " + std::to_string(node->getID()) + " closed the connection");
			break;
		default:
			break;
		}
	}
}

void Simulation::newNodes() {
	const auto& nnds = gui->getNodes();

	for (unsigned int i = nodes.size(); i < nnds.size(); i++) {
		bool goOn = true;
		for (const auto& node : nodes)
			if (node->getID() == nnds[i].index) {
				goOn = false;
			}
		if (nnds[i].local && goOn) {
			/*Creates new node.*/
			if (nnds[i].type == NodeTypes::NEW_FULL)
				nodes.push_back(new Full_Node(io_context, nnds[i].ip, nnds[i].port, nnds[i].index));
			else
				nodes.push_back(new SPV_Node(io_context, nnds[i].ip, nnds[i].port, nnds[i].index));

			/*Sets neighbors.*/
			for (const auto& neighbor : nnds[i].neighbors) {
				auto& ngh = gui->getNode(neighbor);
				nodes.back()->newNeighbor(ngh.index, ngh.ip, ngh.port);
			}
		}
	}
}

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