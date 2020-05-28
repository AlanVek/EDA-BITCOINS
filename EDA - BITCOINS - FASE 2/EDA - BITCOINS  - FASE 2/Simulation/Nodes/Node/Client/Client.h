#pragma once

#include <curl/curl.h>
#include <string>
#include <fstream>
class Client
{
public:
	Client(const std::string&, const std::string&, unsigned int);

	Client();

	void setData(const std::string&, const std::string&, unsigned int);

	void startConnection();

	std::fstream& getBuffer(void);

	~Client();
	void openFile(void);
private:
	void configurateClient(void);
	std::string path, host;
	int port;

	char* contentType;

	std::fstream message;
	CURL* handler;
	CURLcode error;
};
