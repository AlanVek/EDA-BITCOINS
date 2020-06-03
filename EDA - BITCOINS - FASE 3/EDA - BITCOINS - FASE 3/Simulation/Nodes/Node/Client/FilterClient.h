#pragma once
#include "POSTClient.h"

class FilterClient : public POSTClient {
public:
	FilterClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port, const json& data) :
		POSTClient(ip, self_port, out_port, data) {
		url += '/' + indURL;
	}
private:
	const std::string indURL = "send_merkle_block";
};