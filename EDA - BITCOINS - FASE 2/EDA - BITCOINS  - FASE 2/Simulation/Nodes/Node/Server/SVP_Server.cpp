#include "SVP_Server.h"

SVP_Server::SVP_Server(boost::asio::io_context& io_context, const std::string& ip) : Server(io_context, ip)
{
}

void SVP_Server::GETResponse(bool) {
}

void SVP_Server::POSTResponse(bool) {
}

void SVP_Server::errorResponse() {
}