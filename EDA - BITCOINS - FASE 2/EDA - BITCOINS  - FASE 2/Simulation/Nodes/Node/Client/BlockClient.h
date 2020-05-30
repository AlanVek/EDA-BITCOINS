#pragma once
#include "POSTClient.h"
class BlockClient : public POSTClient {
public:

	BlockClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port, const json& data) :
		POSTClient(ip, self_port, out_port, data) {
		url += '/' + indURL;
	}
	~BlockClient() {};
private:
	const std::string indURL = "send_block";
};
