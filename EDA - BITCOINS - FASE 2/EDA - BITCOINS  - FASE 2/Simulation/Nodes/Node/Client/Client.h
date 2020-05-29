#pragma once
#include <curl/curl.h>
#include "json.hpp"
#include <string>

using json = nlohmann::json;

class Client {
public:

	/*Constructor*/
	Client(const std::string& ip, const unsigned int port) : ip(ip), port(port), multiHandler(nullptr), handler(nullptr) {
		if (ip.length() && port)
			stillRunning = 1;
		else
			throw std::exception("Wrong input in client.");
	};

	/*Destructor*/
	virtual ~Client(void) {};

	virtual bool perform() = 0;

protected:
	virtual void configurateClient(void) = 0;

	/*cURL data members.*/
	/**********************/
	CURL* handler, * multiHandler;
	CURLMcode errorMulti;
	/**********************/

	/*Connection data members.*/
	/********************************/
	std::string ip, unparsedAnswer, url;
	json data, answer;
	unsigned int port;
	int stillRunning;
	/********************************/
};
