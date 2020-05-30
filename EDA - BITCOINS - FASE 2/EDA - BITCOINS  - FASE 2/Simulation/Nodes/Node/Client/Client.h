#pragma once
#include <curl/curl.h>
#include "json.hpp"
#include <string>

using json = nlohmann::json;

class Client {
public:

	/*Constructor*/
	Client(const std::string& ip, const unsigned int self_port, const unsigned int out_port);

	/*Destructor*/
	virtual ~Client(void);

	virtual bool perform();

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
	unsigned int self_port, out_port;
	int stillRunning;
	/********************************/
};
