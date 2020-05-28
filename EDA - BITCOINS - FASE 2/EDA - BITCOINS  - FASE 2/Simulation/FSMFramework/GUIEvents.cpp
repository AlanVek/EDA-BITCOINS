#include "GUIEvents.h"

using namespace std;

//

GUIEvents::GUIEvents() {}

genericEvent* GUIEvents::getEvent() {
	unsigned int ev = gui.checkStatus();
	switch (ev) {
		/*User asked to exit.*/
	case Events::END:
		return new End;
		break;
		/*Loads new .json.*/
	//case Events::NEW_FILE:
	//	blockChain.loadBlockChain(gui->getFilename());
	//	gui->setChainLength(blockChain.getBlockAmount());
	//	break;
	//	/*Sets blockID.*/
	//case Events::BLOCKID:
	//	newInfoDispatch(BlockInfo::BLOCKID);
	//	break;
	//	/*Sets MerkleRoot.*/
	//case Events::SEE_MROOT:
	//	newInfoDispatch(BlockInfo::SEE_MROOT);
	//	break;
	//	/*Sets previous blockID.*/
	//case Events::PREVIOUS_BLOCKID:
	//	newInfoDispatch(BlockInfo::PREVIOUS_BLOCKID);
	//	break;
	//	/*Sets nonce.*/
	//case Events::NONCE:
	//	newInfoDispatch(BlockInfo::NONCE);
	//	break;
	//	/*Sets nTx.*/
	//case Events::NTX:
	//	newInfoDispatch(BlockInfo::NTX);
	//	break;
	//	/*Sets block number.*/
	//case Events::BLOCK_NUMBER:
	//	newInfoDispatch(BlockInfo::BLOCK_NUMBER);
	//	break;
	//	/*Sets if the given_mroot is equal to the calculated_mroot.*/
	//case Events::VALIDATE_MROOT:
	//	newInfoDispatch(BlockInfo::VALIDATE_MROOT);
	//	break;
	//	/*Prints tree given in idex.*/
	//case Events::PRINT_TREE:
	//	gui->setInfoShower(blockChain.getBlockInfo(gui->getBlockIndex(), BlockInfo::PRINT_TREE));
	default:
		break;
	}
	return nullptr;
}