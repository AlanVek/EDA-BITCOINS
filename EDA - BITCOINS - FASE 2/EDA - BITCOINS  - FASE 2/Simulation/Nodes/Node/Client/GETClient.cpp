#include "GETClient.h"
#include <iostream>

namespace {
	const char* begURL = "eda_coins";

	//Callback with string as userData.
	size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userData) {
		std::string* userDataPtr = (std::string*) userData;

		userDataPtr->append(ptr, size * nmemb);

		return size * nmemb;
	}
}
GETClient::GETClient(const std::string& ip, const unsigned int port, const std::string& id,
	const unsigned int count) : Client(ip, port), id(id), count(count)
{
	url = ip + '/' + begURL;
}

//Configurates client.
void GETClient::configurateClient(void) {
	unparsedAnswer.clear();

	//Sets handler and multihandler.
	if (curl_multi_add_handle(multiHandler, handler) != CURLE_OK)
		throw std::exception("Failed to set add handler en cURL");

	//Sets URL to read from.
	else if (curl_easy_setopt(handler, CURLOPT_URL, url.c_str()) != CURLE_OK)
		throw std::exception("Failed to set URL in cURL");

	//Sets protocols (HTTP and HTTPS).
	else if (curl_easy_setopt(handler, CURLOPT_PROTOCOLS, CURLPROTO_HTTP) != CURLE_OK)
		throw std::exception("Failed to set HTTP protocol");

	/*Sets port.*/
	else if (curl_easy_setopt(handler, CURLOPT_PORT, port) != CURLE_OK)
		throw std::exception("Failed to set port");

	//Sets callback and userData.
	else if (curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &writeCallback) != CURLE_OK)
		throw std::exception("Failed to set callback");

	else if (curl_easy_setopt(handler, CURLOPT_WRITEDATA, &unparsedAnswer) != CURLE_OK)
		throw std::exception("Failed to set userData");
}

//Performs request.
bool GETClient::perform(void) {
	if (ip.length() && port) {
		static bool step = false;

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
				throw std::exception("Failed to perform cURL.");
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

			answer = json::parse(unparsedAnswer);

			//Sets result to 'FALSE', to end loop.
			stillOn = false;
		}
		return stillOn;
	}
	else
		throw std::exception("Invalid data.");
}