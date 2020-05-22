#pragma once
#include "json.hpp"
#include <list>

using json = nlohmann::json;
class Block
{
public:
	Block(const json&);

	void printData() const;

	const std::string& getMerkleRoot() const;
private:
	void getIDs(const json&);

	/*Block data.*/
	std::list<std::string> nodes, IDs;

	std::string ID, previousID;

	unsigned int  blockNumber, nTx, nonce;

	void getFullData(const json&);

	/*Hashing*/
	static unsigned int generateID(unsigned char* str);
	inline static const std::string hexCodedASCII(unsigned int);
	inline static const std::string hash(const std::string&);

	/*Merkle Tree*/
	void buildTree();
	std::vector<std::string> tree;
	//void printMerkle(void);
	std::string merkleRoot;
};
