#include "POSTClient.h"
#include <iostream>
using json = nlohmann::json;

namespace {
	const char* begURL = "eda_coins";

	//Callback with string as userData.
	size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userData) {
		std::string* userDataPtr = (std::string*) userData;

		userDataPtr->append(ptr, size * nmemb);

		return size * nmemb;
	}
}

//POSTClient constructor.
POSTClient::POSTClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port,
	const json& data) : Client(ip, self_port, out_port), data(data) {
	url = ip + '/' + begURL;
}

//Configurates client.
void POSTClient::configurateClient(void) {
	unparsedAnswer.clear();
	struct curl_slist* list = nullptr;
	//Sets header with token.
	strData = "Data=" + data.dump();
	list = curl_slist_append(list, strData.c_str());

	if (curl_easy_setopt(handler, CURLOPT_POST, 1) != CURLE_OK)
		throw std::exception("Failed to set POST request.");

	//Sets handler and multihandler.
	else if (curl_multi_add_handle(multiHandler, handler) != CURLE_OK)
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

	else if (curl_easy_setopt(handler, CURLOPT_HTTPHEADER, list) != CURLE_OK)
		throw std::exception("Failed to set header.");

	//Sets callback and userData.
	else if (curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &writeCallback) != CURLE_OK)
		throw std::exception("Failed to set callback");

	else if (curl_easy_setopt(handler, CURLOPT_WRITEDATA, &unparsedAnswer) != CURLE_OK)
		throw std::exception("Failed to set userData");
}
POSTClient::~POSTClient() {
}