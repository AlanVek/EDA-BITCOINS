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
	running = gui->nodeSelectionScreen();

	/*If main screen exited successfully...*/
	if (running) {
		/*Loops through every created node.*/
		for (const auto& node : gui->getNodes()) {
			/*Creates new node.*/
			if (node.type == NodeTypes::NEW_FULL)
				nodes.push_back(new Full_Node(io_context, node.ip, node.port, node.index));
			else
				nodes.push_back(new SPV_Node(io_context, node.ip, node.port, node.index));

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
		//nodes[getIndex()]->postFilter(gui->getReceiverID(), gui->getKey(), gui->getAmount());
		gui->infoGotten();

		break;

		/*Blocks (GET).*/
	case Events::GET_BLOCKS:
		ev = Events::GET_BLOCKS;
		nodes[getIndex()]->GETBlocks(gui->getReceiverID(), "84CB2573", 1);
		gui->infoGotten();

		break;

		/*Headers (GET).*/
	case Events::GET_HEADERS:
		ev = Events::GET_HEADERS;
		nodes[getIndex()]->GETBlockHeaders(gui->getReceiverID(), "84CB2573", 1);
		gui->infoGotten();

		break;

		/*Merkleblock (POST).*/
	case Events::MERKLEBLOCK:
		ev = Events::MERKLEBLOCK;
		nodes[getIndex()]->postMerkleBlock(gui->getReceiverID(), "84CB2573", "7B857A14"/*gui->getTransactionID()*/);
		gui->infoGotten();
		break;

		/*Block (POST).*/
	case Events::POST_BLOCK:
		ev = Events::POST_BLOCK;
		nodes[getIndex()]->postBlock(gui->getReceiverID(),/* gui->getBlockID()*/"84CB2573");
		gui->infoGotten();
		break;

		/*Transaction (POST).*/
	case Events::TRANSACTION:
		ev = Events::TRANSACTION;
		nodes[getIndex()]->transaction(gui->getReceiverID(), gui->getWallet(), gui->getAmount());
		gui->infoGotten();

		break;
		/*case Events::BLOCKID:
			gui->setInfoShower(nodes[gui->getCurrentNodeIndex()]->getBlockInfo(gui->getBlockIndex(), BlockInfo::BLOCKID));
			break;
		case Events::SEE_MROOT:
			gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::SEE_MROOT));
			break;
		case Events::PREVIOUS_BLOCKID:
			gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::PREVIOUS_BLOCKID));
			break;
		case Events::NONCE:
			gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::NONCE));
			break;
		case Events::NTX:
			gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::NTX));
			break;
		case Events::BLOCK_NUMBER:
			gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::BLOCK_NUMBER));
			break;
		case Events::VALIDATE_MROOT:
			gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::VALIDATE_MROOT));
			break;
		case Events::PRINT_TREE:
			gui->setInfoShower(blockChain.reprTree(gui->getBlockIndex()));
		default:
			break;*/
	default:
		break;
	}
}

/*Generates event from GUI and polls io_context.*/
const Events Simulation::eventGenerator() {
	io_context.poll();

	for (const auto& node : nodes) {
		switch (node->getClientState()) {
		case ConnectionState::PERFORMING:
			gui->updateMsg("\nNode " + std::to_string(node->getID()) + " is performing a client request.");
			break;
		case ConnectionState::FINISHED:
			gui->updateMsg("\nNode " + std::to_string(node->getID()) + " finished the request.");
			break;
		default:
			break;
		}

		int port_rec = node->getClientPort();
		if (port_rec != -1) {
			gui->updateMsg("\nNode " + std::to_string(node->getID()) + " is answering a request from node " + std::to_string(port_rec));
		}
	}

	return gui->checkStatus();
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