#pragma once
#include "Node/Node.h"
#include "BlockChain/Block/json.hpp"

class SVP_Node : public Node {
public:
	SVP_Node(boost::asio::io_context&, const std::string&, const unsigned int);
	~SVP_Node();

	virtual void newNeighbor(const std::string& ip, const unsigned int port);

protected:
	virtual void NEWGET(const std::string&, const ConnectionType, const std::string&, const unsigned int);
	virtual void NEWPOST(const std::string&, const ConnectionType, const json&);

	virtual void perform();
};
