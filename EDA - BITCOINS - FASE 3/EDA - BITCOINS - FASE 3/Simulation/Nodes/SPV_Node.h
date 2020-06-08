#pragma once
#include "Node/Node.h"
#include "Node/Server/json.hpp"

class SPV_Node : public Node {
public:
	SPV_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int);
	~SPV_Node();

	virtual void perform();
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const unsigned int);
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const std::string&);
	const json& getData() { return headers; }

	virtual const std::string printTree(unsigned int) { return std::string(); };
	virtual const std::string validateMRoot(unsigned int) { return std::string(); };

	virtual const std::string& getKey() { return publicKey; }

	virtual void startTimer() {}

	virtual void checkTimeout(const std::vector < Node*>&) {}

	virtual const std::vector<Neighbor> getAdders() { return std::vector<Neighbor>(0); }

	virtual bool networkDone() { return true; }

private:
	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);

	std::string publicKey;

	nlohmann::json headers;
};
