#pragma once
#include "Node/Node.h"
#include "Node/Server/BlockChain/BlockChain.h"

class Full_Node : public Node {
public:
	Full_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int);
	~Full_Node();

	virtual const std::string GETResponse(const std::string&);
	virtual const std::string POSTResponse(const std::string&);

private:

	const json& getBlock(const std::string& blockID);

	//virtual void transaction(const unsigned int, c)

	virtual void postBlock(const unsigned int, const std::string& blockID);
	virtual void postMerkleBlock(const unsigned int,
		const std::string& blockID, const std::string& transID);

	virtual void postFilter(const unsigned int, const std::string& key, const unsigned int node) {};

	virtual void GETBlocks(const unsigned int, const std::string& blockID, const unsigned int count);
	virtual void GETBlockHeaders(const unsigned int, const std::string& blockID, const unsigned int count) {};

	const json getMerkleBlock(const std::string&, const std::string&);

	BlockChain blockChain;
};
