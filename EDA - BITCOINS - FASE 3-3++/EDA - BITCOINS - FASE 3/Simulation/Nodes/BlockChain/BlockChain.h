#pragma once
#include "Block/Block.h"

using json = nlohmann::json;
class BlockChain
{
public:
	BlockChain(const std::string&);

	BlockChain() {};

	void loadBlockChain(const std::string&);

	static const std::string calculateMerkleRoot(const json& newBlock);

	void addBlock(const json&);

	static const std::string generateID(const std::string& block) { return Block::hash(block); }

	static const std::string calculateBlockID(const json&);

	static const std::string calculateTXID(const json&);

	/*Info getters*/
	/************************************************************/
	const std::string getBlockInfo(int index, const BlockInfo&);
	const unsigned int getBlockAmount() const;
	const json& getBlock(unsigned int);
	unsigned int getBlockIndex(const std::string&);
	const json& getHeader(unsigned int);
	const std::vector<std::string>& getTree(unsigned int);
	const json& getRawData() { return JSON; }
	void newTransaction(const json&);
	/************************************************************/

private:
	std::vector <Block> blocks;
	json JSON;
};
