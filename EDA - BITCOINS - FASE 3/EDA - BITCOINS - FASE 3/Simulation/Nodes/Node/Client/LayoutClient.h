#pragma once
#include "POSTClient.h"
class LayoutClient : public POSTClient {
public:
	LayoutClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port, const json& data) :
		POSTClient(ip, self_port, out_port, data) {
		url += '/' + indURL;
	}
	~LayoutClient() {};
private:
	const std::string indURL = "NETWORK_LAYOUT";
};
