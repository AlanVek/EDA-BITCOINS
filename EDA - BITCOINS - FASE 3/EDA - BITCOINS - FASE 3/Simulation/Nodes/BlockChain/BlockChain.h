#pragma once
#include "Block/Block.h"

using json = nlohmann::json;
class BlockChain
{
public:
	BlockChain(const std::string&);

	void loadBlockChain(const std::string&);

	void addBlock(const json&);

	/*Info getters*/
	/************************************************************/
	const std::string getBlockInfo(int index, const BlockInfo&);
	const unsigned int getBlockAmount() const;
	const json& getBlock(unsigned int);
	unsigned int getBlockIndex(const std::string&);
	const json& getHeader(unsigned int);
	const std::vector<std::string>& getTree(unsigned int);
	const json& getRawData() { return JSON; }
	/************************************************************/

private:
	std::vector <Block> blocks;
	json JSON;
};
