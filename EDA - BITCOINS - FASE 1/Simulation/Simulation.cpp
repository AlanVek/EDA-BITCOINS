#include "Simulation.h"
#include <iostream>
#include <functional>

using namespace std::placeholders;

//Simulation constructor.
Simulation::Simulation(void) : running(true)
{
	/*Attempts to create new GUI variable.*/
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
		gui->setChainLength(blockChain.getBlockAmount());
		break;
	case Events::BLOCKID:
		gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::BLOCKID));
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