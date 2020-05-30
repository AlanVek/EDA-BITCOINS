#pragma once

#include "GETClient.h"
class GETBlockClient : public GETClient {
public:

	GETBlockClient(const std::string& ip, const unsigned int self_port, unsigned int out_port, const std::string id, const unsigned int count)
		: GETClient(ip, self_port, out_port, id, count) {
		url += '/' + indURL + "?block_id=" + id + "&count=" + std::to_string(count);
	}
	~GETBlockClient() {};
private:
	const std::string indURL = "get_blocks";
};