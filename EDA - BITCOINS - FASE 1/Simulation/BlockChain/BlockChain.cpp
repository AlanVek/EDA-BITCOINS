#include "BlockChain.h"
#include <fstream>
#include <iostream>

BlockChain::BlockChain() {}

BlockChain::BlockChain(const std::string& filename) {
	loadBlockChain(filename);
}

void BlockChain::loadBlockChain(const std::string& filename) {
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

void BlockChain::printMerkleRoots() {
	for (const auto& block : blocks)
		std::cout << block.getMerkleRoot() << std::endl;
}

void BlockChain::printBlockData(const std::initializer_list<unsigned int>& indexes) {
	if (indexes.size()) {
		for (auto index : indexes) {
			if (index >= 0 && index < blocks.size())
				blocks[index].printData();
		}
	}
	else {
		for (const auto& block : blocks) {
			block.printData();
		}
	}
}