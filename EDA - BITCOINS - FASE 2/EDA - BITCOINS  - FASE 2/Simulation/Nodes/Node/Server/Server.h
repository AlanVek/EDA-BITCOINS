#pragma once
#include <boost/asio.hpp>
#include <string>
#include <functional>

#define MAXSIZE 10000
namespace {
	using Response = std::function<const std::string(const std::string&)>;
}
class Server
{
public:
	Server(boost::asio::io_context&, const std::string&, const Response&, const Response&);
	virtual ~Server();
protected:

	const enum class Connections {
		NONE = 0,
		POST,
		GET
	};

	/*Connection methods.*/
	/*******************************************/
	void asyncConnection(void);
	void closeConnection(void);

	void answer(const std::string&);
	Response GETResponse;
	Response POSTResponse;
	virtual void errorResponse(void);
	/*******************************************/

	/*Callbacks and callback-related.*/
	/*********************************************************************************/
	void connectionCallback(const boost::system::error_code& error);
	void messageCallback(const boost::system::error_code& error, size_t bytes_sent);
	void inputValidation(const boost::system::error_code& error, size_t bytes);
	/*********************************************************************************/

	/*Boost::asio data members.*/
	/****************************************/
	boost::asio::io_context& io_context;
	boost::asio::ip::tcp::acceptor acceptor;
	boost::asio::ip::tcp::socket socket;
	/****************************************/

	/*Connection data members.*/
	/*********************************************/
	size_t size;
	char mess[MAXSIZE];
	std::string response, host;
	Connections state;
	/*********************************************/
};
