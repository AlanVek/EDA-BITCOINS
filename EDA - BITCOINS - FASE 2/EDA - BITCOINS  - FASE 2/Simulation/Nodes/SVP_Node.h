#pragma once
#include "Node/Node.h"
#include "Node/Server/json.hpp"

class SVP_Node : public Node {
public:
	SVP_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int);
	~SVP_Node();

	virtual void transaction(const unsigned int, const std::string& wallet, const unsigned int amount);

	virtual void postBlock(const unsigned int, const std::string& blockID) {};
	virtual void postMerkleBlock(const unsigned int,
		const std::string& blockID, const std::string& transID) {};

	virtual void postFilter(const unsigned int, const std::string& key, const unsigned int node);

	virtual void GETBlocks(const unsigned int, const std::string& blockID, const unsigned int count) {};
	virtual void GETBlockHeaders(const unsigned int, const std::string& blockID, const unsigned int count);

private:

	virtual const std::string GETResponse(const std::string&, unsigned int node_id);
	virtual const std::string POSTResponse(const std::string&, unsigned int node_id);

	nlohmann::json headers;
};
