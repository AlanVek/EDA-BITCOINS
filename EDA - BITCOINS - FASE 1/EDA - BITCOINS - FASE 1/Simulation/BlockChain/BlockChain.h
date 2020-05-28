#pragma once
#include "Block/Block.h"

using json = nlohmann::json;
class BlockChain
{
public:
	BlockChain();
	BlockChain(const std::string&);

	void loadBlockChain(const std::string& filename);

	const std::string getBlockInfo(int index, const BlockInfo&);

	const unsigned int getBlockAmount() const;

	const std::string reprTree(unsigned int);

private:
	std::vector <Block> blocks;
	json JSON;
};
