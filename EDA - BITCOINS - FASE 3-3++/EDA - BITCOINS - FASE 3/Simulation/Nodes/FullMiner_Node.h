#pragma once
#include "Full_Node.h"
class FullMiner_Node : public Full_Node
{
public:
	FullMiner_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int, int&);
	virtual ~FullMiner_Node();

	virtual void perform();

	virtual const json getFeeTrans();
protected:

	void mineBlock(void);
};
