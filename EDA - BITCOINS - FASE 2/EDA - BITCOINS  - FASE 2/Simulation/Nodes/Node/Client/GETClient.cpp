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
GETClient::GETClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port, const std::string& id,
	const unsigned int count) : Client(ip, self_port, out_port), id(id), count(count)
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

	/*Sets port that receives request.*/
	else if (curl_easy_setopt(handler, CURLOPT_PORT, out_port) != CURLE_OK)
		throw std::exception("Failed to set receiving port");

	/*Sets port that sends request.*/
	else if (curl_easy_setopt(handler, CURLOPT_LOCALPORT, self_port) != CURLE_OK)
		throw std::exception("Failed to set sending port");

	//Sets callback and userData.
	else if (curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &writeCallback) != CURLE_OK)
		throw std::exception("Failed to set callback");

	else if (curl_easy_setopt(handler, CURLOPT_WRITEDATA, &unparsedAnswer) != CURLE_OK)
		throw std::exception("Failed to set userData");
}