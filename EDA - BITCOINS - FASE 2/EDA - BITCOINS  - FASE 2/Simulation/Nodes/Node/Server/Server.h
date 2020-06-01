#pragma once
#include <boost/asio.hpp>
#include <string>
#include <functional>

#define MAXSIZE 100000

namespace {
	using Response = std::function<const std::string(const std::string&, const boost::asio::ip::tcp::endpoint&)>;
	using errorResp = std::function<const std::string(void)>;
}
class Server
{
public:
	Server(boost::asio::io_context&, const std::string&, const Response&, const Response&, const errorResp&, unsigned int);
	virtual ~Server();

protected:
	void newConnector(void);

	const enum class ConnectionTypes {
		NONE = 0,
		POST,
		GET
	};
	struct Connection {
		Connection(boost::asio::io_context& io_context) : socket(io_context) {}
		boost::asio::ip::tcp::socket socket;
		char reader[MAXSIZE];
		std::string response;
		std::list<Connection>::iterator pos;
	};

	/*Connection methods.*/
	/*******************************************/
	void asyncConnection(Connection&);
	void closeConnection(Connection&);

	void answer(Connection&, const std::string&);
	Response GETResponse;
	Response POSTResponse;
	errorResp errorResponse;
	/*******************************************/

	/*Callbacks and callback-related.*/
	/*********************************************************************************/
	void connectionCallback(Connection&, const boost::system::error_code& error);
	void messageCallback(Connection&, const boost::system::error_code& error, size_t bytes_sent);
	void inputValidation(Connection&, const boost::system::error_code& error, size_t bytes);
	/*********************************************************************************/

	/*Boost::asio data members.*/
	/****************************************/
	boost::asio::io_context& io_context;
	boost::asio::ip::tcp::acceptor acceptor;
	std::list<Connection> sockets;
	/****************************************/

	/*Connection data members.*/
	/*********************************************/
	size_t size;
	std::string host;
	ConnectionTypes state;
	unsigned int port;
	/*********************************************/
};
