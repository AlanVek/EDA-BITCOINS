#pragma once
#include "Block/Block.h"

using json = nlohmann::json;
class BlockChain
{
public:
	BlockChain();
	//BlockChain(const std::string&);

	//void loadBlockChain(const std::string& filename);

	void addBlock(const json&);

	const std::string getBlockInfo(int index, const BlockInfo&);

	const unsigned int getBlockAmount() const;

	const json& getBlock(unsigned int);

	unsigned int getBlockIndex(const std::string&);
	const json& getHeader(unsigned int);

private:
	std::vector <Block> blocks;
	json JSON;
};
