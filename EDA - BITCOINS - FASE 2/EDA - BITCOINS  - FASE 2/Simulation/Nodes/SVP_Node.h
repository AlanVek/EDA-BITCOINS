#pragma once
#include "Node/Node.h"
class SVP_Node : public Node {
public:
	SVP_Node(boost::asio::io_context&);
	~SVP_Node();

protected:
	virtual void connect(const std::string&, unsigned int);
	virtual void connectionCallback();
};
