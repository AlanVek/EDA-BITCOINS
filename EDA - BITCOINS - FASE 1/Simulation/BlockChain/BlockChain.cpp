#include "BlockChain.h"
#include <fstream>

BlockChain::BlockChain() {}

BlockChain::BlockChain(const std::string& filename) {
	loadBlockChain(filename);
}

void BlockChain::loadBlockChain(const std::string& filename) {
	blocks.clear();

	/*Attempts to open file.*/
	std::fstream jsonFile(filename, std::ios::in);

	if (!jsonFile.is_open()) {
		jsonFile.close();
		throw std::exception("Failed to open file.");
	}

	/*Parses file input.*/
	JSON = json::parse(jsonFile);

	jsonFile.close();

	/*For every block in the blockChain...*/
	for (auto& j : JSON)
		blocks.push_back(Block(j));
}

const unsigned int BlockChain::getBlockAmount() const { return blocks.size(); }

const std::string BlockChain::reprTree(unsigned int index) {
	return blocks[index].printTree();
};

const std::string BlockChain::getBlockInfo(int index, const BlockInfo& member) {
	return blocks[index].getData(member);
}