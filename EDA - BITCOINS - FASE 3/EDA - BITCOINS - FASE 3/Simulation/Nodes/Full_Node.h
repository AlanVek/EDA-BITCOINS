#pragma once
#include "Node/Node.h"
#include "BlockChain/BlockChain.h"

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
private:
	const json getMerkleBlock(const std::string&, const std::string&);
	const json& getBlock(const std::string& blockID);

	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);

	BlockChain blockChain;
};
