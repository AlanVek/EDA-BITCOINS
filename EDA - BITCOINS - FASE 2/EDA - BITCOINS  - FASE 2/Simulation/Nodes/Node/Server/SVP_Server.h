#pragma once
#include "Server.h"
#include "json.hpp"

using json = nlohmann::json;

class SVP_Server : public Server
{
public:
	SVP_Server(boost::asio::io_context&, const std::string&);

	const json& getHeaders() {};

protected:
	virtual void GETResponse(bool);
	virtual void POSTResponse(bool);
	virtual void errorResponse(void);

	json headers;
};
