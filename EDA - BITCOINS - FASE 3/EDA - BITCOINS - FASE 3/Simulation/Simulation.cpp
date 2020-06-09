#include "Simulation.h"
#include "Nodes/Full_Node.h"
#include "Nodes/SPV_Node.h"

//Simulation constructor.
Simulation::Simulation(void) : running(true), ev(Events::NOTHING), size(0)
{
	/*Attempts to create new GUI variable.*/
	gui = new GUI(nodes);

	srand(time(NULL));
}

/*Calls main GUI screen for node selection.*/
void Simulation::mainScreen() {
	bool newNet;
	running = gui->nodeSelectionScreen(&newNet);

	/*If main screen exited successfully...*/
	if (running) {
		/*Sets nodes and creates connections.*/
		if (newNet) {
			newNodes(false);

			/*CREATE NEW NETWORK.*/

			for (auto& node : nodes) {
				node->startTimer();
			}
		}

		/*Justs sets nodes. Connections are already determined.*/
		else {
			newNodes(true);
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
		//gui->setRealNodes(nodes);
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

		/*New nodes have been added, so the nodes vector
		must be updated.*/
	case Events::UPDATE:

		/*Sets new nodes.*/
		newNodes(true);

		/*For non-blocking network creation.*/
	case Events::KEEPCREATING:

		if (createNetwork()) {
			gui->networkDone();
		}

		break;
	default:
		break;
	}
}

/*Generates event from GUI and polls io_context.*/
const Events Simulation::eventGenerator() {
	/*Polls io_context.*/
	io_context.poll();

	/*Sets networking message in GUI.*/
	generateMsg();

	/*Generates and returns event.*/
	return gui->checkStatus();
}

/*Generates and sets message in GUI.*/
void Simulation::generateMsg() {
	/*Vector to block from setting the same string when client is taking too long.*/
	static std::vector<bool> canPrint(nodes.size(), true);

	/*Updates vector's size in case new nodes have been added.*/
	if (nodes.size() > canPrint.size()) {
		std::vector<bool>temp(nodes.size() - canPrint.size(), true);
		canPrint.insert(canPrint.end(), temp.begin(), temp.end());
	}

	/*For every node...*/
	for (unsigned int i = 0; i < nodes.size(); i++) {
		/*Sets string according to client state.*/
		for (auto& state : nodes[i]->getClientState()) {
			switch (state) {
				/*Performing string.*/
			case ClientState::PERFORMING:
				if (canPrint[i]) {
					gui->updateMsg("\nNode " + std::to_string(nodes[i]->getID()) + " is performing a client request.");
					canPrint[i] = false;
				}
				break;

				/*Finished string.*/
			case ClientState::FINISHED:
				gui->updateMsg("\nNode " + std::to_string(nodes[i]->getID()) + " finished the request.");
				canPrint[i] = true;
				break;
			default:
				break;
			}
		}
		auto serverStates = nodes[i]->getServerState();
		auto ports = nodes[i]->getClientPort();
		for (unsigned int j = 0; j < serverStates.size(); j++)
			/*Sets string according to server state.*/
			switch (serverStates[j].st) {
				int client_reception;

				/*Connection OK string.*/
			case ServerState::PERFORMING:

				/*With known neighbor.*/
				if ((client_reception = ports[j])) {
					gui->updateMsg("\nNode " + std::to_string(nodes[i]->getID()) + " is answering a request from node " + std::to_string(client_reception));
				}
				else
					gui->updateMsg("\nNode " + std::to_string(nodes[i]->getID()) + " is answering a request from an unknown node.");
				break;
				/*Finished string.*/
			case ServerState::FINISHED:
				if (ports.size() && ports.back() + 1) {
					gui->updateMsg("\nNode " + std::to_string(nodes[i]->getID()) + " answered a request from node " + std::to_string(ports.back()));
				}
				else
					gui->updateMsg("\nNode " + std::to_string(nodes[i]->getID()) + " answered a request from an unknown node.");
				break;
			default:
				break;
			}
	}
}

/*Sets new nodes in nodes vector.*/
void Simulation::newNodes(bool request) {
	const auto& nnds = gui->getNodes();

	/*Goes from the last index of the current vector to the last index
	of the updated vector.*/
	for (unsigned int i = nodes.size(); i < nnds.size(); i++) {
		bool goOn = true;

		/*Checks if node's ID is already in the vector.
		If it is, it sets the flag to false, so the rest of the
		function doesn't load it again.*/
		for (const auto& node : nodes) {
			if (node->getPort() == nnds[i].port && node->getIP() == nnds[i].ip) {
				goOn = false;
			}
		}
		/*If it's a local node and isn't already in the nodes vector...*/
		if (nnds[i].local && goOn) {
			/*Creates new node.*/
			if (nnds[i].type == NodeTypes::NEW_FULL)
				nodes.push_back(new Full_Node(io_context, nnds[i].ip, nnds[i].port, nnds[i].index, size));
			else
				nodes.push_back(new SPV_Node(io_context, nnds[i].ip, nnds[i].port, nnds[i].index, size));

			/*If it was created from appendix mode, it must request (BLOCK if it's a FULL or HEADER if it's an SPV).
			Parameters "0" and NULL mean "all the blocks/headers". */
			if (request) {
				/*Sets neighbors.*/
				for (const auto& neighbor : nnds[i].neighbors) {
					auto& ngh = gui->getNode(neighbor);
					bool added = false;
					for (auto& node : nodes) {
						if (node->getIP() == ngh.ip && node->getPort() == ngh.port) {
							nodes.back()->newNeighbor(node->getID(), ngh.ip, ngh.port);
							node->newNeighbor(nodes.back()->getID(), nodes.back()->getIP(), nodes.back()->getPort());
							added = true;
						}
					}
					if (!added) {
						nodes.back()->newNeighbor(ngh.index, ngh.ip, ngh.port);
					}
				}

				if (nnds[i].type == NodeTypes::NEW_FULL) {
					nodes.back()->perform(ConnectionType::GETBLOCK, (*nodes.back()->getNeighbors().begin()).first, "0", NULL);
					nodes.back()->perform(ConnectionType::PING, NULL, (*nodes.back()->getNeighbors().begin()).second.ip, (*nodes.back()->getNeighbors().begin()).second.port);
				}
				else {
					nodes.back()->perform(ConnectionType::GETHEADER, (*nodes.back()->getNeighbors().begin()).first, "0", NULL);

					for (const auto& neighbor : nodes.back()->getNeighbors()) {
						nodes.back()->perform(ConnectionType::POSTFILTER, neighbor.first, nodes.back()->getKey(), NULL);
						nodes.back()->perform(ConnectionType::PING, NULL, neighbor.second.ip, neighbor.second.port);
					}
				}
			}
		}
	}
	size = nnds.size();
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

bool Simulation::createNetwork() {
	bool done = true;

	perform();
	generateMsg();

	for (auto& node : nodes) {
		node->checkTimeout(nodes);

		if (!node->networkDone())
			done = false;
	}

	if (done) {
		connectSPVs();
		addAdders();
	}

	return done;
}

void Simulation::connectSPVs() {
	int tempIndex;
	for (auto& node : nodes) {
		if (typeid(*node) == typeid(SPV_Node)) {
			while (node->getNeighbors().size() < 2) {
				auto neighbor = nodes[rand() % nodes.size()];

				if (typeid(*neighbor) != typeid(SPV_Node)) {
					node->newNeighbor(neighbor->getID(), neighbor->getIP(), neighbor->getPort());
					neighbor->newNeighbor(node->getID(), node->getIP(), node->getPort());
				}
			}
		}
	}
}

void Simulation::addAdders() {
	for (auto& node : nodes) {
		for (auto& neighbor : node->getAdders()) {
			for (auto& node_2 : nodes) {
				if (node_2->getIP() == neighbor.ip && node_2->getPort() == neighbor.port) {
					node->newNeighbor(node_2->getID(), neighbor.ip, neighbor.port);
				}
			}
		}
	}
}