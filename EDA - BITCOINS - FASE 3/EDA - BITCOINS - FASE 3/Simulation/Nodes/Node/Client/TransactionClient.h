#pragma once
#include "POSTClient.h"

class TransactionClient : public POSTClient {
public:
	TransactionClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port,
		const json& data) : POSTClient(ip, self_port, out_port, data) {
		url += '/' + indURL;
	}
private:
	const std::string indURL = "send_tx";
};
