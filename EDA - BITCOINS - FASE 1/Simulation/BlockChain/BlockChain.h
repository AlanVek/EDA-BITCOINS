#pragma once
#include "Block/Block.h"

using json = nlohmann::json;
class BlockChain
{
public:
	BlockChain();
	BlockChain(const std::string&);

	void loadBlockChain(const std::string& filename);

	void printMerkleRoots(void);

	void printBlockData(const std::initializer_list<unsigned int> & = {});

	const std::string getBlockInfo(int index, const BlockInfo&) const;

	const unsigned int getBlockAmount() const;

private:

	json JSON;

	std::vector <Block> blocks;
};
