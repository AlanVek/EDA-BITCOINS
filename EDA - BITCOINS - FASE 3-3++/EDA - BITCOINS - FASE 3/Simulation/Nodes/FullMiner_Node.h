#pragma once
#include "Full_Node.h"
class FullMiner_Node : public Full_Node
{
public:
	FullMiner_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int, int&);
	virtual ~FullMiner_Node();

	virtual void perform();

	virtual const json getFeeTrans(bool);

	/*Sets info (if necessary) and queues new action.*/
	virtual void perform(ConnectionType type, const unsigned int id, const std::string& blockID, const unsigned int count);

protected:

	void mineBlock(bool);
};
