#pragma once
#include <list>
#include "json.hpp"

using json = nlohmann::json;

class jsonHandler {
public:
	jsonHandler();

	jsonHandler(const char*);

	void newJSON(const char* filename);

private:
	static const std::string hexCodedASCII(unsigned int);
	void getIDs(const json&);
	void buildMerkle(void);
	std::list<std::string> nodes;
	static unsigned int generateID(unsigned char* str);
	json JSON, tree;

	std::list<std::string> IDs;
};
