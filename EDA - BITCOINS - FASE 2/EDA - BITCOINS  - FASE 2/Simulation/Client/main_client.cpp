#include <iostream>
#include "Client.h"
#include "DataInput.h"
#include "Errors.h"

int main(int argC, char** argV) {
	Location userData;
	int result = -1;

	if (getData(argV, argC, &userData)) {
		try {
			//Creates instance of client.
			Client myNewClient(userData.host, userData.path, 80);

			//Begins connection to host.
			myNewClient.startConnection();

			std::cout << "Successfully connected with server.\n";

			result = 0;
		}
		catch (Error& e) {
			std::cout << e.what() << std::endl;
		}
	}
	else
		std::cout << "Failed to reach server. Wrong syntax.\n";

	return result;
}