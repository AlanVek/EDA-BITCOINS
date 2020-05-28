#include <iostream>
#include "DataInput.h"

//Expects two parameters from command line: executable's name and host/path/filename.
#define EXPECTEDPARAMS 2

/*Separates data into host and path/filename. Returns true if data is valid, otherwise it returns false.*/
bool getData(char** argV, int argC, void* userData) {
	Location* userDataPtr = (Location*)userData;

	bool result = false;
	std::string data;
	int pos;

	//If the amount of arguments is correct...
	if (argC == EXPECTEDPARAMS) {
		data = argV[1];
		result = true;

		//Searches for '/' symbol.
		pos = data.find('/');

		/*If there isn't one, then the whole input is the host.
		Manipulates data string so that path is an empty string*/
		if (pos == std::string::npos) {
			pos = data.length();
			data.append(" ");
		}

		//Sets host and path in userData.
		userDataPtr->host = data.substr(0, pos);
		userDataPtr->path = data.substr(pos + 1, data.length() - pos + 1);
	}

	return result;
}