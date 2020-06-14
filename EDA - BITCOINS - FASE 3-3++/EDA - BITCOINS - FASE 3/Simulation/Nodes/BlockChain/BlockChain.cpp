#include "BlockChain.h"
#include <fstream>

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

/*Gets blockchain's size.*/
const unsigned int BlockChain::getBlockAmount() const { return blocks.size(); }

/*Gets block info by index and type.*/
const std::string BlockChain::getBlockInfo(int index, const BlockInfo& member) {
	return blocks[index].getData(member);
}

/*Gets block by index.*/
const json& BlockChain::getBlock(unsigned int index) { return blocks[index].block; }

/*Adds block to blockchain.*/
void BlockChain::addBlock(const json& block) {
	blocks.push_back(Block(block));

	JSON.push_back(block);
}

const std::vector<std::string>& BlockChain::getTree(unsigned int index) {
	if (!blocks[index].tree.size())
		blocks[index].buildTree();

	return blocks[index].tree;
};

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

void BlockChain::newTransaction(const json& trans) {
	blocks.back().block["tx"].push_back(trans);
	blocks.back().tx.push_back(trans);
}

const std::string BlockChain::calculateMerkleRoot(const json& newBlock) {
	Block temp(newBlock);
	temp.getData(BlockInfo::VALIDATE_MROOT);
	return temp.calculatedMerkleRoot;
}

const std::string BlockChain::calculateBlockID(const json& block) {
	Block temp(block);
	auto& header = temp.header;

	std::string result;

	if (header.find("blockid") != header.end())
		header.erase("blockid");

	for (auto& member : header) {
		result.append(member.get<std::string>());
	}

	return Block::hash(result);
};

const std::string BlockChain::calculateTXID(const json& trans) {
	std::string result;
	for (auto& input : trans["vin"]) {
		result.append(input["txid"].get<std::string>());
	}

	return Block::hash(result);
};