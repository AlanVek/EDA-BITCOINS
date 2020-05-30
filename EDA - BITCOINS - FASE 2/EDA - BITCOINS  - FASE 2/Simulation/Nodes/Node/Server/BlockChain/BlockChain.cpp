#include "BlockChain.h"
#include <fstream>

BlockChain::BlockChain() {}

//BlockChain::BlockChain(const std::string& filename) {
//	loadBlockChain(filename);
//}
//
//void BlockChain::loadBlockChain(const std::string& filename) {
//	blocks.clear();
//
//	/*Attempts to open file.*/
//	std::fstream jsonFile(filename, std::ios::in);
//
//	if (!jsonFile.is_open()) {
//		jsonFile.close();
//		throw std::exception("Failed to open file.");
//	}
//
//	/*Parses file input.*/
//	JSON = json::parse(jsonFile);
//
//	jsonFile.close();
//
//	/*For every block in the blockChain...*/
//	for (auto& j : JSON)
//		blocks.push_back(Block(j));
//}

const unsigned int BlockChain::getBlockAmount() const { return blocks.size(); }

const std::string BlockChain::getBlockInfo(int index, const BlockInfo& member) {
	return blocks[index].getData(member);
}

const json& BlockChain::getBlock(unsigned int index) { return blocks[index].block; }

void BlockChain::addBlock(const json& block) { blocks.push_back(Block(block)); }
unsigned int BlockChain::getBlockIndex(const std::string& blockID) {
	for (unsigned int i = 0; i < blocks.size(); i++) {
		if (blocks[i].header["blockid"] == blockID)
			return i;
	}
	return -1;
}

const json& BlockChain::getHeader(unsigned int index) { return blocks[index].header; }