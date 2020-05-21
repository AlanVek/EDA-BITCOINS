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
	newJSON(filename);

	getIDs();

	buildMerkle();
}

void jsonHandler::newJSON(const char* filename) {
	std::fstream jsonFile(filename, std::ios::in);

	if (!jsonFile.is_open()) {
		jsonFile.close();
		throw std::exception("Failed to open file.");
	}

	JSON = json::parse(jsonFile);

	jsonFile.close();
}

unsigned int jsonHandler::generateID(unsigned char* str) {
	unsigned int ID = 0;
	int c;
	while (c = *str++)
		ID = c + (ID << 6) + (ID << 16) - ID;
	return ID;
}

void jsonHandler::getIDs(void) {
	IDs.clear();
	std::string ss;
	unsigned int tempID;

	for (const auto& j : JSON) {
		/*If JSON has 'vin' key...*/
		if (j.find("tx") != j.end()) {
			if (j["tx"][0].find("vin") != j["tx"][0].end()) {
				/*Loops through every 'mini JSON' in JSON['vin'].*/
				for (const auto& miniJson : j["tx"][0]["vin"]) {
					/*Validates existence of 'txid'.*/
					if (miniJson.find("txid") != miniJson.end()) {
						/*Gets string from JSON.*/
						ss = miniJson["txid"].get<std::string>();

						/*Transforms string to numerical ID.*/
						tempID = generateID((unsigned char*)ss.c_str());

						/*Transforms numerical ID to hex Coded ASCII.*/
						IDs.push_back(hexCodedASCII(tempID));
					}
					else
						throw std::exception("Wrong JSON format. Expected 'txid' identifier.");
				}

				/*Duplicates last entry if there's an uneven amount of entries.*/
				if (IDs.size() % 2)
					IDs.push_back(IDs.back());
			}
			else
				throw std::exception("Wrong JSON format. Expected 'vin' identifier.");
		}
		else
			throw std::exception("Wrong JSON format. Excepted 'tx' identifier.");
	}
}

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

void jsonHandler::buildMerkle(std::list<std::string>::iterator* itr) {
	static std::list<std::string> nodes(IDs);
	static bool going = true;

	std::list<std::string>::iterator itrTemp;

	if (!itr) {
		itr = new std::list<std::string>::iterator(nodes.begin());
	}

	std::string temp;

	if ((*itr) != nodes.end() && going) {
		if ((*itr) == nodes.end() || ++(*itr) == nodes.end()) {
			going = false;
			(*itr)--;
		}
		else {
			temp = **itr;
			(*itr)--;
			(**itr).append(temp);
			**itr = hexCodedASCII(generateID((unsigned char*)temp.c_str()));
			itrTemp = ++(*itr);
			(*itr)--;
			nodes.erase(itrTemp);
		}
	}

	if (nodes.size() > 1)
		buildMerkle(itr);

	else {
		std::cout << *nodes.begin() << std::endl;
		nodes.clear();
		if (itr)
			delete itr;
	}
}