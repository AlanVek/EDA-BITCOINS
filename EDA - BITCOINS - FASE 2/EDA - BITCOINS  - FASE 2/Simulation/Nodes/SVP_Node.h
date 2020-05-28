#pragma once
#include "Node/Node.h"
#include "BlockChain/Block/json.hpp"
class SVP_Node : public Node {
public:
	SVP_Node(boost::asio::io_context&);
	~SVP_Node();

protected:
	virtual void connect(const std::string&, const unsigned int);
	virtual void newNeighbor(const std::string& ip, const unsigned int port);
};
