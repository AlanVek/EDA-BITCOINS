#pragma once
#include "Node/Node.h"
#include "Node/Server/json.hpp"

class SVP_Node : public Node {
public:
	SVP_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int);
	~SVP_Node();

	//virtual void newNeighbor(const unsigned int, const std::string& ip, const unsigned int port);

	virtual void perform();

	virtual const unsigned int& getID();

	//virtual void transaction(const unsigned int, c)

	virtual void postBlock(const unsigned int, const std::string& blockID) {};
	virtual void postMerkleBlock(const unsigned int,
		const std::string& blockID, const std::string& transID) {};

	virtual void postFilter(const unsigned int, const std::string& key, const unsigned int node) {};

	virtual void GETBlocks(const unsigned int, const std::string& blockID, const unsigned int count) {};
	virtual void GETBlockHeaders(const unsigned int, const std::string& blockID, const unsigned int count) {};

	virtual const States getState(void);

private:

	virtual const std::string GETResponse(const std::string&);
	virtual const std::string POSTResponse(const std::string&);

	nlohmann::json headers;
};
