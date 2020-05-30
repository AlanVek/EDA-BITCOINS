#include "POSTClient.h"

using json = nlohmann::json;

namespace {
	const char* begURL = "eda_coin";

	//Callback with string as userData.
	size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userData) {
		std::string* userDataPtr = (std::string*) userData;

		userDataPtr->append(ptr, size * nmemb);

		return size * nmemb;
	}
}

//POSTClient constructor.
POSTClient::POSTClient(const std::string& ip, const unsigned int port, const json& data) : Client(ip, port), data(data) {
	url = ip + '/' + begURL;
}

//Configurates client for tweet request.
void POSTClient::configurateClient(void) {
	unparsedAnswer.clear();

	struct curl_slist* list = nullptr;

	//Sets handler and multihandler.
	curl_multi_add_handle(multiHandler, handler);

	//Sets URL to read from.
	curl_easy_setopt(handler, CURLOPT_URL, url.c_str());

	//Tells cURL to redirect if requested.
	curl_easy_setopt(handler, CURLOPT_FOLLOWLOCATION, 1L);

	//Sets protocols (HTTP and HTTPS).
	curl_easy_setopt(handler, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);

	//Sets header with token.
	list = curl_slist_append(list, ("Data: " + data.dump()).c_str());
	curl_easy_setopt(handler, CURLOPT_HTTPHEADER, list);

	//Sets callback and userData.
	curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &writeCallback);
	curl_easy_setopt(handler, CURLOPT_WRITEDATA, &unparsedAnswer);
}

//Gets tweets.
bool POSTClient::perform(void) {
	if (ip.length() && port) {
		static bool step = false;

		bool stillOn = true;

		if (!step) {
			//Sets easy and multi modes with error checker.
			handler = curl_easy_init();
			if (!handler)
				throw std::exception("Failed to initialize easy handler.");

			multiHandler = curl_multi_init();

			if (!multiHandler)
				throw std::exception("Failed to initialize multi handler.");

			//If it's the first time in this run, it sets the request parameters.
			configurateClient();
			step = true;
		}

		//Should be an if. Performs one request and checks for errors.
		if (stillRunning) {
			errorMulti = curl_multi_perform(multiHandler, &stillRunning);
			if (errorMulti != CURLE_OK) {
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

POSTClient::~POSTClient() {}