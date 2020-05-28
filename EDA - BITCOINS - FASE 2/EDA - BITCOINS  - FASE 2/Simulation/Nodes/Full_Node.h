#pragma once
#include "Node/Node.h"
#include "BlockChain/BlockChain.h"

class Full_Node : public Node {
public:
	Full_Node(boost::asio::io_context&);
	~Full_Node();

protected:
	virtual void connect(const std::string&, const unsigned int);
	virtual void newNeighbor(const std::string& ip, const unsigned int port);

	BlockChain blockChain;
};
