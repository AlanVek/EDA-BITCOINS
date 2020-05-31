#pragma once
#include <boost/asio.hpp>
#include <string>
#include <functional>

#define MAXSIZE 100000

struct Connection {
	Connection(boost::asio::io_context& io_context) : socket(io_context) {}
	boost::asio::ip::tcp::socket socket;
	char reader[MAXSIZE];
	std::string response;
	std::list<Connection>::iterator pos;
};

namespace {
	using Response = std::function<const std::string(const std::string&, const unsigned int)>;

	using iterator = Connection&;
}
class Server
{
public:
	Server(boost::asio::io_context&, const std::string&, const Response&, const Response&, unsigned int);
	virtual ~Server();

protected:
	void newConnector(void);

	const enum class Connections {
		NONE = 0,
		POST,
		GET
	};

	/*Connection methods.*/
	/*******************************************/
	void asyncConnection(iterator);
	void closeConnection(iterator);

	void answer(iterator, const std::string&);
	Response GETResponse;
	Response POSTResponse;
	const std::string errorResponse(void);
	/*******************************************/

	/*Callbacks and callback-related.*/
	/*********************************************************************************/
	void connectionCallback(iterator, const boost::system::error_code& error);
	void messageCallback(iterator, const boost::system::error_code& error, size_t bytes_sent);
	void inputValidation(iterator, const boost::system::error_code& error, size_t bytes);
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
	Connections state;
	unsigned int port;
	/*********************************************/
};
