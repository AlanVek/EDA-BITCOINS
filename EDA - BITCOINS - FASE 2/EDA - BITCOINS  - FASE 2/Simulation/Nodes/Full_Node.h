#pragma once
#include "Node/Node.h"
#include "BlockChain/BlockChain.h"

class Full_Node : public Node {
public:
	Full_Node(boost::asio::io_context&);
	~Full_Node();

protected:
	virtual void NEWGET(const std::string&, const ConnectionType, const std::string&, const unsigned int);
	virtual void NEWPOST(const std::string&, const ConnectionType, const json&);

	virtual void perform();

	virtual void newNeighbor(const std::string& ip, const unsigned int port);

	BlockChain blockChain;
};
