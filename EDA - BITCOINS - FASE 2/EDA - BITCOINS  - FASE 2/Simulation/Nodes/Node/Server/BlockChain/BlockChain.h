#pragma once
#include "Block/Block.h"

using json = nlohmann::json;
class BlockChain
{
public:
	BlockChain();

	void addBlock(const json&);

	/*Info getters*/
	/************************************************************/
	const std::string getBlockInfo(int index, const BlockInfo&);
	const unsigned int getBlockAmount() const;
	const json& getBlock(unsigned int);
	unsigned int getBlockIndex(const std::string&);
	const json& getHeader(unsigned int);
	/************************************************************/

private:
	std::vector <Block> blocks;
	json JSON;
};
