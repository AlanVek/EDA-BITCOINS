#pragma once

#include <boost/asio.hpp>
#include <string>

#define MAXSIZE 10000

class Server
{
public:
	Server(boost::asio::io_context& io_context_);
	~Server();
private:

	/*Connection methods.*/
	/*******************************************/
	void waitForConnection(void);
	void closeConnection(void);

	void answer(bool isInputOk);
	std::string generateTextResponse(bool);
	size_t getFileSize(std::fstream& file);
	/*******************************************/

	/*Callbacks and callback-related.*/
	/*********************************************************************************/
	void connectionCallback(const boost::system::error_code& error);
	void messageCallback(const boost::system::error_code& error, size_t bytes_sent);
	void inputValidation(const boost::system::error_code& error, size_t bytes);
	/*********************************************************************************/

	/*Data members.*/
	/*********************************************/
	boost::asio::io_context& io_context;
	boost::asio::ip::tcp::acceptor acceptor;
	boost::asio::ip::tcp::socket socket;

	size_t size;
	char mess[MAXSIZE];
	std::string response;
	/*********************************************/
};
