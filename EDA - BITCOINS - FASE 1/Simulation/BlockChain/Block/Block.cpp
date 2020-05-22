#include "Block.h"
#include <iostream>

Block::Block(const json& JSON) {
	this->JSON = JSON;
	transformData();

	getIDs();

	/*Copies IDs vector to temporary list 'nodes'.*/
	nodes.assign(IDs.begin(), IDs.end());

	/*Builds tree*/
	buildTree();
};

void Block::transformData() {
	JSON["height"] = std::to_string(JSON["height"].get<unsigned int>());
	JSON["nonce"] = std::to_string(JSON["nonce"].get<unsigned int>());
	JSON["nTx"] = std::to_string(JSON["nTx"].get<unsigned int>());
}

/*Gets transaction IDs from json.*/
void Block::getIDs() {
	IDs.clear();

	unsigned int tempID;

	if (JSON.is_null())
		return;

	std::string tx_id;
	/*For every transaction...*/
	for (const auto& TX : JSON["tx"]) {
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
		JSON["calculated_merkleRoot"] = nodes.back();
		if (nodes.back() == JSON["merkleroot"])
			JSON["validation"] = "True";
		else
			JSON["validation"] = "False";
	}
}

const std::string Block::getData(const BlockInfo& data) const {
	switch (data) {
	case BlockInfo::BLOCKID:
		return JSON["blockid"];
	case BlockInfo::BLOCK_NUMBER:
		return JSON["height"];
	case BlockInfo::SEE_MROOT:
		return JSON["merkleroot"];
	case BlockInfo::VALIDATE_MROOT:
		return JSON["validation"];
	case BlockInfo::NTX:
		return JSON["nTx"];
	case BlockInfo::NONCE:
		return JSON["nonce"];
	case BlockInfo::PREVIOUS_BLOCKID:
		return JSON["previousblockid"];
	}
}

std::string Block::printTree(void) const {
	unsigned int levels = log2(tree.size() + 1);

	std::string result;
	unsigned int temp;
	unsigned int templevel = levels;
	for (int i = 0; i < tree.size(); i++) {
		temp = pow(2, templevel - 1);
		if (i % temp || !i)
			result.append(tree[i] + '\t');
		else {
			result.append("\n\t" + tree[i]);
			templevel--;
		}
	}
	return result;
}