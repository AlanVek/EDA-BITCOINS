#pragma once
#include <list>
#include "json.hpp"

using json = nlohmann::json;

class jsonHandler {
public:
	jsonHandler();

	jsonHandler(const std::string&);

	void newJSON(const std::string& filename);

private:
	inline static const std::string hexCodedASCII(unsigned int);
	void getIDs(const json&);
	void printMerkle(void);
	static unsigned int generateID(unsigned char* str);
	json JSON;

	inline static const std::string hash(const std::string&);

	std::list<std::string> nodes, IDs;
	std::vector<std::string> tree;

	void buildTree();
};
