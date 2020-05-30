#include "Full_Server.h"

Full_Server::Full_Server(boost::asio::io_context& io_context, const std::string& ip)
	: Server(io_context, ip)
{
}

void Full_Server::GETResponse(bool) {
}

void Full_Server::POSTResponse(bool) {
}

void Full_Server::errorResponse() {
}