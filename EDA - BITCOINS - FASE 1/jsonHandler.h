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
	json JSON, tree;

	void getIDs(const json&);

	static const std::string hexCodedASCII(unsigned int);
	void buildMerkle(void);
	std::list<std::string> IDs;
	std::list<std::string> nodes;
	static unsigned int generateID(unsigned char* str);
};
