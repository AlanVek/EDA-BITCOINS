#pragma once
#include <boost/asio.hpp>
#include <string>
#include <functional>

#define MAXSIZE 10000
#define MAXNEIGHBORS 10
namespace {
	using Response = std::function<const std::string(const std::string&)>;
}
class Server
{
public:
	Server(boost::asio::io_context&, const std::string&, const Response&, const Response&);
	virtual ~Server();

	void newNeighbor(void);

protected:

	const enum class Connections {
		NONE = 0,
		POST,
		GET
	};

	/*Connection methods.*/
	/*******************************************/
	void asyncConnection(unsigned int);
	void closeConnection(unsigned int);

	void answer(unsigned int, const std::string&);
	Response GETResponse;
	Response POSTResponse;
	virtual void errorResponse(void);
	/*******************************************/

	/*Callbacks and callback-related.*/
	/*********************************************************************************/
	void connectionCallback(unsigned int, const boost::system::error_code& error);
	void messageCallback(const boost::system::error_code& error, size_t bytes_sent);
	void inputValidation(unsigned int, const boost::system::error_code& error, size_t bytes);
	/*********************************************************************************/

	/*Boost::asio data members.*/
	/****************************************/
	boost::asio::io_context& io_context;
	boost::asio::ip::tcp::acceptor acceptor;
	std::vector<boost::asio::ip::tcp::socket> sockets;
	/****************************************/

	/*Connection data members.*/
	/*********************************************/
	size_t size;
	char mess[MAXNEIGHBORS][MAXSIZE];
	std::vector<std::string> responses;
	std::string host;
	Connections state;
	/*********************************************/
};
