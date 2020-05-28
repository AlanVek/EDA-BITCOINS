#include "Client.h"
#include <iostream>
#include <curl/easy.h>
#include <fstream>
#include "Errors.h"

const int fileNameSize = 10;

//Callback for when message is received.
size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userData);

//Client constructor. Initializes CURL, and the easy mode. Calls client configuration.
Client::Client(const std::string& host, const std::string& path, unsigned int port) : host(host), path(path), port(port)
{
	handler = nullptr;

	error = curl_global_init(CURL_GLOBAL_ALL);

	if (error == CURLE_OK) {
		handler = curl_easy_init();
		if (handler)
			configurateClient();
		else
			throw (Error("Failed to initialize handler.\n"));
	}
	else
		throw(Error("Failed to set Curl.\n"));
}

Client::Client() {
	handler = nullptr;
};

//Client constructor. Initializes CURL, and the easy mode. Calls client configuration.
void Client::setData(const std::string& host, const std::string& path, unsigned int port) {
	this->host = host;
	this->path = path;
	this->port = port;

	error = curl_global_init(CURL_GLOBAL_ALL);

	if (error == CURLE_OK) {
		handler = curl_easy_init();
		if (handler)
			configurateClient();
		else
			throw (Error("Failed to initialize handler.\n"));
	}
	else
		throw(Error("Failed to set Curl.\n"));
}

//Client destructor.
Client::~Client() {
	curl_easy_cleanup(handler);
	curl_global_cleanup();
	message.close();
}

//After initial setup, activates the handler.
void Client::startConnection(void) {
	if (handler) {
		error = curl_easy_perform(handler);
		if (error != CURLE_OK)
			throw(Error("Bad connection.\n"));
	}
	else
		throw std::exception("Client data not loaded.");
}

/*Configurates client to send a GET request to server and
write the data gathered with write_callback into "this" client as userData.*/
void Client::configurateClient(void) {
	curl_easy_setopt(handler, CURLOPT_URL, (host + '/' + path).c_str());
	curl_easy_setopt(handler, CURLOPT_PORT, port);
	curl_easy_setopt(handler, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
	curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &writeCallback);
	curl_easy_setopt(handler, CURLOPT_WRITEDATA, this);
}

//Write callback. Appends incoming message to this->message.
size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userData) {
	Client* userDataPtr = (Client*)userData;

	std::fstream& output = userDataPtr->getBuffer();

	if (!output.is_open())
		userDataPtr->openFile();

	output.write(ptr, nmemb);

	return size * nmemb;
}

//Message getter.
std::fstream& Client::getBuffer(void) { return message; }

void Client::openFile(void) {
	std::string filename;
	std::string ctnt;
	int pos = path.find_last_of('.');

	auto removeBar = [](std::string& name) {std::string temp; for (auto x : name) { if (x != '/') temp += x; } return temp; };

	error = curl_easy_getinfo(handler, CURLINFO_CONTENT_TYPE, &contentType);

	if (error == CURLE_OK) {
		ctnt = contentType;

		if (pos == std::string::npos || pos == path.length())
			filename = removeBar(path);

		else {
			filename = path.substr(0, pos);
			filename = removeBar(filename);
		}

		if (filename.length() > fileNameSize)
			filename = filename.substr(filename.length() - fileNameSize, fileNameSize);

		pos = ctnt.find('/');
		ctnt = ctnt.substr(pos + 1, ctnt.length() - pos);

		message.open((filename + '.' + ctnt).c_str(), std::ios::out | std::ios::binary);
		if (!message.is_open()) {
			message.close();
			throw(Error("Failed to create output file.\n"));
		}
	}
	else
		throw(Error("Failed to get content type.\n"));
}