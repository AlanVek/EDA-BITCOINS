#pragma once
#include "Server.h"
#include "BlockChain/BlockChain.h"
class Full_Server : public Server
{
public:
	Full_Server(boost::asio::io_context&, const std::string&);

protected:
	virtual void GETResponse(bool);
	virtual void POSTResponse(bool);
	virtual void errorResponse();

	BlockChain blockChain;
};
