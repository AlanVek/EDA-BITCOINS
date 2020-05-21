#include "jsonHandler.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace {
	const char hexLimit_inf = 'a';
	const char hexLimit_sup = 'f';
}

jsonHandler::jsonHandler() {};

jsonHandler::jsonHandler(const char* filename) {
	/*Performs to get Merkle Tree.*/
	newJSON(filename);
}

void jsonHandler::newJSON(const char* filename) {
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
		buildMerkle();
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
	std::string ss;
	unsigned int tempID;

	if (Json.is_null())
		return;

	/*For every transaction...*/
	for (const auto& TX : Json["tx"]) {
		/*Loops through every 'mini JSON' in JSON['vin'].*/
		for (const auto& miniJson : TX["vin"]) {
			/*Gets string from JSON.*/
			ss = miniJson["txid"].get<std::string>();

			/*Transforms string to numerical ID.*/
			tempID = generateID((unsigned char*)ss.c_str());

			/*Transforms numerical ID to hex Coded ASCII.*/
			IDs.push_back(hexCodedASCII(tempID));
		}
	}
}

/*Transforms int into hex Coded ASCII.*/
const std::string jsonHandler::hexCodedASCII(unsigned int number) {
	std::stringstream stre;

	/*Generates HEX number from int.*/
	stre << std::setfill('0') << std::setw(2 * sizeof(unsigned int)) << std::hex << number;

	/*Sets string with result.*/
	std::string result = stre.str();

	/*Transforms string to upper-case.*/
	for (int i = 0; i < result.length(); i++) {
		if (hexLimit_inf <= result[i] && hexLimit_sup >= result[i])
			result[i] = std::toupper(result[i]);
	}

	return result;
}

/*Gets Merkle Root.*/
void jsonHandler::buildMerkle(void) {
	static bool going = true;

	std::list<std::string>::iterator itrTemp;

	/*While nodes list is not the Merkle Root...*/
	while (nodes.size() > 1) {
		/*If node amount is uneven, it copies the last one to the back of the list.*/
		if (nodes.size() % 2)
			nodes.push_back(nodes.back());

		/*For every node in the list...*/
		for (auto i = nodes.begin(); i != nodes.end(); i++) {
			itrTemp = ++i;

			/*Concats next node's content to the current node's content.*/
			(*(--i)).append(*itrTemp);

			/*Transforms content to ID and ID to hex Coded ASCII.*/
			*i = hexCodedASCII(generateID((unsigned char*)(*i).c_str()));

			/*Erases next node.*/
			nodes.erase(itrTemp);
		}
	}

	if (nodes.size())
		std::cout << "Merkle root: " << nodes.front() << std::endl;
}