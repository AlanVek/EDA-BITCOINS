#pragma once
#include "POSTClient.h"
class BlockClient : public POSTClient {
public:

	BlockClient(const std::string& ip, const unsigned int port, const json& data) : POSTClient(ip, port, data) {
		url += '\\' + indURL;
	}
	~BlockClient() {};
private:
	const std::string indURL = "send_block";
};
