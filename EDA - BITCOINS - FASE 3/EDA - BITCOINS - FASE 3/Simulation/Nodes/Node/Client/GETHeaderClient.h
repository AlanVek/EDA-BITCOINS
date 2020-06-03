#pragma once
#include "GETClient.h"
class GETHeaderClient : public GETClient {
public:

	GETHeaderClient(const std::string& ip, const unsigned int self_port, const unsigned int out_port, const std::string id, const unsigned int count)
		: GETClient(ip, self_port, out_port, id, count) {
		url += '/' + indURL + "?block_id=" + id + "&count=" + std::to_string(count);
	}
	~GETHeaderClient() {};
private:
	const std::string indURL = "get_block_header";
};
