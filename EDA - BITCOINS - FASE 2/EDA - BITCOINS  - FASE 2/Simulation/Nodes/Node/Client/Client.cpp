#include "Client.h"
#include <iostream>

Client::Client(const std::string& ip, const unsigned int self_port, const unsigned int out_port) : ip(ip), self_port(self_port),
out_port(out_port), multiHandler(nullptr), handler(nullptr) {
	if (ip.length() && self_port && out_port)
		stillRunning = 1;
	else
		throw std::exception("Wrong input in client.");
};

//Performs request.
bool Client::perform(void) {
	if (ip.length() && self_port && out_port) {
		static bool step = false;
		std::cout << "Performing." << std::endl;

		bool stillOn = true;

		if (!step) {
			//Sets easy and multi modes with error checker.
			if (!(handler = curl_easy_init()))
				throw std::exception("Failed to initialize easy handler.");

			if (!(multiHandler = curl_multi_init()))
				throw std::exception("Failed to initialize multi handler.");

			//If it's the first time in this run, it sets the request parameters.
			configurateClient();
			step = true;
		}

		//Should be an if. Performs one request and checks for errors.
		if (stillRunning) {
			if (curl_multi_perform(multiHandler, &stillRunning) != CURLE_OK) {
				curl_easy_cleanup(handler);
				curl_multi_cleanup(multiHandler);
				handler = nullptr;
				multiHandler = nullptr;
				throw std::exception("Failed to perform cURL.");
			}
		}
		else {
			//Cleans used variables.
			curl_easy_cleanup(handler);
			curl_multi_cleanup(multiHandler);
			handler = nullptr;
			multiHandler = nullptr;

			//Resets step to false.
			step = false;

			//Resets stillRunning to 1;
			stillRunning = 1;

			//Parses answer.

			answer = json::parse(unparsedAnswer);

			//Sets result to 'FALSE', to end loop.
			stillOn = false;
		}
		return stillOn;
	}
	else
		throw std::exception("Invalid data.");
}

Client::~Client() {}