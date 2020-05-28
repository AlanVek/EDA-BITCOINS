#pragma once
#include "POSTClient.h"

class MerkleClient : public POSTClient {
public:
	MerkleClient(const std::string& ip, const unsigned int port, const json& data) : POSTClient(ip, port, data) {
		url += '/' + indURL;
	}
private:
	const std::string indURL = "send_merkle_block";
};