#include "Client.h"
#include <iostream>

Client::Client(const std::string& ip, const unsigned int self_port, const unsigned int out_port) : ip(ip), self_port(self_port),
out_port(out_port), multiHandler(nullptr), handler(nullptr) {
	if (ip.length() && self_port && out_port)
		stillRunning = 1;
	else
		throw std::exception("Wrong input in client.");

	url = ip + '/' + begURL;
};

//Performs request.
bool Client::perform(void) {
	bool stillOn = true;
	static bool step = false;
	try {
		if (ip.length() && self_port && out_port) {
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
				if (curl_multi_perform(multiHandler, &stillRunning) != CURLM_OK) {
					curl_easy_cleanup(handler);
					curl_multi_cleanup(multiHandler);
					handler = nullptr;
					multiHandler = nullptr;
					throw std::exception("Failed to connect.");
				}
			}

			else {
				//Cleans used variables.
				curl_easy_cleanup(handler);
				curl_multi_cleanup(multiHandler);

				//Resets step to false.
				step = false;

				//Resets stillRunning to 1;
				stillRunning = 1;

				//Parses answer.
				try {
					answer = json::parse(unparsedAnswer);
				}
				catch (std::exception&) {
					json excpt;
					excpt["status"] = false;
					excpt["error"] = "Invalidad Data. Wrong JSON format.";
					throw std::exception(excpt.dump().c_str());
				}
				std::cout << "Received: " << answer << std::endl;

				//Sets result to 'FALSE', to end loop.
				stillOn = false;
			}
		}
		else
			throw std::exception("Invalid data.");
	}
	catch (std::exception& e) {
		answer = json::parse(e.what());
		stillOn = false;
		step = false;
		stillRunning = 1;
		std::cout << "Received: " << answer << std::endl;
	}
	return stillOn;
}

size_t Client::writeCallback(char* ptr, size_t size, size_t nmemb, void* userData) {
	std::string* userDataPtr = (std::string*) userData;

	userDataPtr->append(ptr, size * nmemb);

	return size * nmemb;
}

const json& Client::getAnswer() {
	return answer;
}

Client::~Client() {}