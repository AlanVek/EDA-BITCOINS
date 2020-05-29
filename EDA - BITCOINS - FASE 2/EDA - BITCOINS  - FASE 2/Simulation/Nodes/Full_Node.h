#pragma once
#include "Node/Node.h"

class Full_Node : public Node {
public:
	Full_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int);
	~Full_Node();

	virtual void newNeighbor(const unsigned int, const std::string& ip, const unsigned int port);

	virtual void perform();
	virtual const unsigned int& getID();

	virtual void NEWGET(const unsigned int&, const ConnectionType, const std::string&, const unsigned int);
	virtual void NEWPOST(const unsigned int&, const ConnectionType, const json&);

	virtual const States getState(void);
};
