#include "Block.h"
#include <iostream>

Block::Block(const json& JSON) {
	getIDs(JSON);

	getFullData(JSON);

	/*Copies IDs vector to temporary list 'nodes'.*/
	nodes.assign(IDs.begin(), IDs.end());

	/*Builds tree*/
	buildTree();
};

void Block::getFullData(const json& JSON) {
	blockNumber = JSON["height"].get<unsigned int>();
	ID = JSON["blockid"].get<std::string>();
	previousID = JSON["previousblockid"].get<std::string>();
	nonce = JSON["nonce"].get<unsigned int>();
	nTx = JSON["nTx"].get<unsigned int>();
}

/*Gets transaction IDs from json.*/
void Block::getIDs(const json& Json) {
	IDs.clear();

	unsigned int tempID;

	if (Json.is_null())
		return;

	std::string tx_id;
	/*For every transaction...*/
	for (const auto& TX : Json["tx"]) {
		/*Loops through every 'mini JSON' in JSON['vin'].*/
		for (const auto& miniJson : TX["vin"])
			/*Gets string from JSON.*/
			tx_id.append(miniJson["txid"].get<std::string>());

		/*Hashes id and appends it to IDs.*/
		IDs.push_back(hash(tx_id));

		tx_id.clear();
	}
}

unsigned int Block::generateID(unsigned char* str) {
	unsigned int ID = 0;
	int c;
	while (c = *str++)
		ID = c + (ID << 6) + (ID << 16) - ID;
	return ID;
}
/*Transforms int into hex Coded ASCII.*/
inline const std::string Block::hexCodedASCII(unsigned int number) {
	char res[9];
	sprintf_s(res, "%08X", number);

	return res;
}

inline const std::string Block::hash(const std::string& code) {
	return hexCodedASCII(generateID((unsigned char*)code.c_str()));
}

void Block::buildTree(void) {
	static bool going = true;

	std::list<std::string>::iterator itrTemp;

	/*While nodes list is not the Merkle Root...*/
	while (nodes.size() > 1) {
		/*If node amount is uneven, it copies the last one to the back of the list.*/
		if (nodes.size() % 2)
			nodes.push_back(nodes.back());

		tree.insert(tree.end(), nodes.begin(), nodes.end());

		/*For every node in the list...*/
		for (auto i = nodes.begin(); i != nodes.end(); i++) {
			itrTemp = ++i;

			/*Concats next node's content to the current node's content.*/
			(*(i)) = hash(*(--i)) + hash(*itrTemp);

			/*Hashes node's content.*/
			*i = hash(*i);

			/*Erases next node.*/
			nodes.erase(itrTemp);
		}
	}
	if (nodes.size()) {
		tree.insert(tree.end(), nodes.back());
		merkleRoot = nodes.back();
	}
}
const std::string& Block::getMerkleRoot() const { return merkleRoot; }

void Block::printData() const {
	std::cout << "{\n";
	std::cout << "\tBlock ID: " << ID << std::endl;
	std::cout << "\tHeight: " << blockNumber << std::endl;
	std::cout << "\tMerkleRoot " << merkleRoot << std::endl;
	std::cout << "\tnTx: " << nTx << std::endl;
	std::cout << "\tNonce: " << nonce << std::endl;
	std::cout << "\tPrevious block ID:  " << previousID << std::endl;
	std::cout << "}\n";
}