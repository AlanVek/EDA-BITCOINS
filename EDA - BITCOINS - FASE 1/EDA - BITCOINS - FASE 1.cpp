#include <iostream>
#include "jsonHandler.h"

#include <list>

std::initializer_list<std::string> nns = { "A","B","C","D", "E", "F", "G", "H" };
std::list <std::string> nodes = nns;
void numerillo();

int main()
{
	try {
		jsonHandler j;

		j.newJSON("blockChain.json");

		//numerillo();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void numerillo() {
	static bool going = true;

	std::list<std::string>::iterator itrTemp;
	std::string temp;

	/*For every pair in the list...*/
	for (auto i = nodes.begin(); i != nodes.end() && going; i++)

		/*If it's the last one, it leaves.*/
		if (i == nodes.end() || ++i == nodes.end()) {
			going = false;
			i--;
		}

	/*Otherwise, it concats both strings and gets
	a new ID and a new hex Coded ASCII from that ID.*/
		else {
			temp = *i;
			i--;
			(*i).append(temp);
			itrTemp = (++i);
			i--;
			nodes.erase(itrTemp);
		}

	if (nodes.size() > 1)
		numerillo();

	else {
		std::cout << *nodes.begin() << std::endl;
	}
}