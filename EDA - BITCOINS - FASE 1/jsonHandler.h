#pragma once
#include <list>
#include "json.hpp"

using json = nlohmann::json;

class jsonHandler {
public:
	jsonHandler();
	jsonHandler(const char*);
	void newJSON(const char* filename);

	static const std::string hexCodedASCII(unsigned int);

private:
	json JSON, tree;
	std::list<std::string> IDs;
	void getIDs(void);

	static unsigned int generateID(unsigned char* str);

	void buildMerkle(std::list<std::string>::iterator* = nullptr);
};
