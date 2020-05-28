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
	VALIDATE_MROOT,
	PRINT_TREE
};

using json = nlohmann::json;
class Block
{
public:
	Block(const json&);

	friend BlockChain;

	~Block();

private:

	std::string printTree();

	const std::string getData(const BlockInfo&);

	const std::list < std::string> getIDs();

	/*Block data.*/
	json header, tx;

	void transformData();

	/*Hashing*/
	static unsigned int generateID(unsigned char* str);
	inline static const std::string hexCodedASCII(unsigned int);
	inline static const std::string hash(const std::string&);

	/*Merkle Tree*/
	void buildTree();
	std::vector<std::string> tree;
	std::string calculatedMerkleRoot;
	std::string isMROK;
};
