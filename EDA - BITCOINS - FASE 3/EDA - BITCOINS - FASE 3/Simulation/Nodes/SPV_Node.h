#pragma once
#include "Node/Node.h"
#include "Node/Server/json.hpp"

class SPV_Node : public Node {
public:
	SPV_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int);
	~SPV_Node();

	virtual void transaction(const unsigned int, const std::string& wallet, const unsigned int amount);

	virtual void postBlock(const unsigned int, const std::string& blockID) {};
	virtual void postMerkleBlock(const unsigned int, const std::string& blockID, const std::string& transID) {};

	virtual void postFilter(const unsigned int, const std::string& key);

	virtual void GETBlocks(const unsigned int, const std::string& blockID, const unsigned int count) {};
	virtual void GETBlockHeaders(const unsigned int, const std::string& blockID, const unsigned int count);

	virtual void perform();

private:
	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);

	nlohmann::json headers;
};
