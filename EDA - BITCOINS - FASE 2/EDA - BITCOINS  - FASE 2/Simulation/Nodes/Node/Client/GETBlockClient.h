#pragma once

#include "GETClient.h"
class GETBlockClient : public GETClient {
public:

	GETBlockClient(const std::string& ip, const unsigned int port, const std::string id, const unsigned int count)
		: GETClient(ip, port, id, count) {
		url += '/' + indURL + "?block_id=" + id + "&count=" + std::to_string(count);
	}
	~GETBlockClient() {};
private:
	const std::string indURL = "get_blocks";
};