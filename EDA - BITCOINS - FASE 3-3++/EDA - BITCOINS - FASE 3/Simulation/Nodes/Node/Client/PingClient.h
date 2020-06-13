#pragma once
#include "POSTClient.h"
class PingClient : public POSTClient
{
public:
	PingClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port, const json& data) :
		POSTClient(ip, self_port, out_port, data) {
		url += '/' + indURL;
	}
	~PingClient() {};
private:
	const std::string indURL = "PING";
};
