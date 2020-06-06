#pragma once
#include "Node/Node.h"
#include "BlockChain/BlockChain.h"

/*****Full node events********/

const enum class NodeEvents : unsigned int {
	NOTHING = 0,
	PING,
	TIMEOUT,
	NETWORK_NOT_READY,
	NETWORK_READY,
	NETWORK_LAYOUT,
	END
};

/*****Full node states********/

const enum class NodeState : unsigned int {
	IDLE = 0,
	WAITING_LAYOUT,
	COLLECTING_MEMBERS,
	NETWORK_CREATED,
	END
};

class Full_Node : public Node {
public:
	Full_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int);
	~Full_Node();

	virtual void perform(ConnectionType, const unsigned int, const std::string&, const unsigned int);
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const std::string&);

	virtual void perform();

	virtual const json& getData() {
		return blockChain.getRawData();
	}

	virtual const std::string printTree(unsigned int idx) { return blockChain.getBlockInfo(idx, BlockInfo::PRINT_TREE); }
	virtual const std::string validateMRoot(unsigned int idx) { return blockChain.getBlockInfo(idx, BlockInfo::VALIDATE_MROOT); }

	virtual void nodeDispatcher(void);		/*****************************************/
	virtual NodeEvents nodeEventGenerator(void);		/**************************************/

	virtual const std::string& getKey() { return std::string(); }

private:
	const json getMerkleBlock(const std::string&, const std::string&);
	const json& getBlock(const std::string& blockID);

	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);

	BlockChain blockChain;
	NodeEvents ev;
	NodeState state;
};
