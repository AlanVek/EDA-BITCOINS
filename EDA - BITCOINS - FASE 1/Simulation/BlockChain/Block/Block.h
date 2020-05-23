#pragma once
#include "json.hpp"
#include <list>

class BlockChain;

const enum class BlockInfo {
	SEE_MROOT,
	BLOCKID,
	PREVIOUS_BLOCKID,
	NTX,
	BLOCK_NUMBER,
	NONCE,
	VALIDATE_MROOT
};

using json = nlohmann::json;
class Block
{
public:
	Block(const json&);

	std::string printTree();

	const std::string getData(const BlockInfo&);
private:

	void getIDs();

	json JSON;

	/*Block data.*/
	std::list<std::string> nodes, IDs;

	void transformData();

	/*Hashing*/
	static unsigned int generateID(unsigned char* str);
	inline static const std::string hexCodedASCII(unsigned int);
	inline static const std::string hash(const std::string&);

	/*Merkle Tree*/
	void buildTree();
	std::vector<std::string> tree;
};
