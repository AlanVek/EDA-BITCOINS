#include "BlockChain.h"
#include <fstream>

BlockChain::BlockChain() {}

/*Gets blockchain's size.*/
const unsigned int BlockChain::getBlockAmount() const { return blocks.size(); }

/*Gets block info by index and type.*/
const std::string BlockChain::getBlockInfo(int index, const BlockInfo& member) {
	return blocks[index].getData(member);
}

/*Gets block by index.*/
const json& BlockChain::getBlock(unsigned int index) { return blocks[index].block; }

/*Adds block to blockchain.*/
void BlockChain::addBlock(const json& block) { blocks.push_back(Block(block)); }

/*Gets block index by id.*/
unsigned int BlockChain::getBlockIndex(const std::string& blockID) {
	for (unsigned int i = 0; i < blocks.size(); i++) {
		if (blocks[i].header["blockid"] == blockID)
			return i;
	}
	return -1;
}

/*Gets block's header by id.*/
const json& BlockChain::getHeader(unsigned int index) { return blocks[index].header; }