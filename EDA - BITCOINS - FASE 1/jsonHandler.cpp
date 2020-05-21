#include "jsonHandler.h"
#include <fstream>
#include <iostream>

namespace {
	const char hexLimit_inf = 'a';
	const char hexLimit_sup = 'f';
}

jsonHandler::jsonHandler() {}

jsonHandler::jsonHandler(const std::string& filename) {
	/*Performs to get Merkle Tree.*/
	newJSON(filename);
}

void jsonHandler::newJSON(const std::string& filename) {
	/*Attempts to open file.*/
	std::fstream jsonFile(filename, std::ios::in);

	if (!jsonFile.is_open()) {
		jsonFile.close();
		throw std::exception("Failed to open file.");
	}

	/*Parses file input.*/
	JSON = json::parse(jsonFile);

	jsonFile.close();

	/*For every block in the blockChain...*/
	for (auto& j : JSON) {
		/*Gets transaction IDs.*/
		getIDs(j);

		/*Copies IDs vector to temporary list 'nodes'.*/
		nodes.assign(IDs.begin(), IDs.end());

		/*Gets Merkle Root and prints it.*/
		buildTree();

		//printMerkle();

		tree.clear();
	}
}

unsigned int jsonHandler::generateID(unsigned char* str) {
	unsigned int ID = 0;
	int c;
	while (c = *str++)
		ID = c + (ID << 6) + (ID << 16) - ID;
	return ID;
}

/*Gets transaction IDs from json.*/
void jsonHandler::getIDs(const json& Json) {
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

/*Transforms int into hex Coded ASCII.*/
inline const std::string jsonHandler::hexCodedASCII(unsigned int number) {
	char res[9];
	sprintf_s(res, "%08X", number);

	return res;
}

///*Gets Merkle Root.*/
//void jsonHandler::printMerkle(void) {
//	unsigned int levels = log2(tree.size() + 1);
//
//	for (int i = levels; i > -1; i--) {
//		for (int j = 0; j < pow(2, levels - i); j++) {
//			/*for (int k = 0; k < pow(2, i + 1); k++)
//				std::cout << ' ';*/
//			std::cout << tree[pow(2, i)
//		} j] << ' ';
//		}
//		std::cout << std::endl;
//	}
//}

void jsonHandler::buildTree(void) {
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
		std::cout << "Merkle root: " << nodes.back() << std::endl;
	}
}

inline const std::string jsonHandler::hash(const std::string& code) {
	return hexCodedASCII(generateID((unsigned char*)code.c_str()));
}