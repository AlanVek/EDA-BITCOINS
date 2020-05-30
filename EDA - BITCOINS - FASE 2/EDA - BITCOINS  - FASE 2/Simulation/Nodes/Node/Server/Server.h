#pragma once
#include <boost/asio.hpp>
#include <string>
#include <functional>

#define MAXSIZE 10000
#define MAXNEIGHBORS 10

struct Connection {
	boost::asio::ip::tcp::socket socket;
	//char message[MAXSIZE];
	std::string response;
	std::string reader;
};

namespace {
	using Response = std::function<const std::string(const std::string&)>;

	using iterator = const std::list<Connection>::iterator&;
}
class Server
{
public:
	Server(boost::asio::io_context&, const std::string&, const Response&, const Response&);
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
	void messageCallback(const boost::system::error_code& error, size_t bytes_sent);
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
	/*********************************************/
};
