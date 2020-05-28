#pragma once
#include "POSTClient.h"

class TransactionClient : public POSTClient {
public:
	TransactionClient(const std::string& ip, const unsigned int port, const json& data) : POSTClient(ip, port, data) {
		url += '/' + indURL;
	}
private:
	const std::string indURL = "send_tx";
};
