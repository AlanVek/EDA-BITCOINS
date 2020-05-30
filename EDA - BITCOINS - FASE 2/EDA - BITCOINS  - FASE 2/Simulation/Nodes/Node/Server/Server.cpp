#include "Server.h"
#include <iostream>
#include <boost\bind.hpp>
#include <fstream>

using boost::asio::ip::tcp;

namespace {
	const std::string fixed = "eda_coin";
}

/*Server constructor. Initializes io_context, acceptor and socket.
Calls asyncConnection to accept connections.*/
Server::Server(boost::asio::io_context& io_context_, const std::string& host, const Response& GET,
	const Response& POST) : host(host), io_context(io_context_),
	acceptor(io_context_, tcp::endpoint(tcp::v4(), 80))
{
	GETResponse = GET;
	POSTResponse = POST;
}

void Server::newNeighbor() {
	sockets.push_back(boost::asio::ip::tcp::socket(io_context));
	responses.push_back(std::string());
	if (sockets.back().is_open()) {
		sockets.back().shutdown(tcp::socket::shutdown_both);
		sockets.back().close();
	}
	asyncConnection(sockets.back(), sockets.size() - 1);
}

//Destructor. Closes open socket and acceptor.
Server::~Server() {
	std::cout << "\nClosing server.\n";

	for (auto& socket : sockets) {
		if (socket.is_open()) {
			socket.shutdown(tcp::socket::shutdown_both);
			socket.close();
		}
	}

	if (acceptor.is_open())
		acceptor.close();

	std::cout << "Server is closed.\n";
}

/*Sets acceptor to accept (asynchronously).*/
void Server::asyncConnection(boost::asio::ip::tcp::socket& socket, unsigned int index) {
	if (acceptor.is_open()) {
		std::cout << "Waiting for connection.\n";
		if (!socket.is_open()) {
			acceptor.async_accept(
				socket, boost::bind(&Server::connectionCallback,
					this, boost::ref(socket), index, boost::asio::placeholders::error)
			);
		}
		responses[index].clear();
	}
}

//Closes socket and clears message holder.
void Server::closeConnection(boost::asio::ip::tcp::socket& socket, unsigned int index) {
	socket.shutdown(tcp::socket::shutdown_both);
	socket.close();
	int i = 0;
	for (unsigned int i = 0; i < MAXSIZE; i++) {
		mess[index][i] = NULL;
	}
}

/*Validates input given in GET request.*/
void Server::inputValidation(boost::asio::ip::tcp::socket& socket, unsigned int index, const boost::system::error_code& error, size_t bytes) {
	if (!error) {
		//Creates string message from request.
		std::string message(mess[index]);

		//Validator has the http protocol form.
		std::string validator_GET = "GET /" + fixed + '/';
		std::string validator_POST = "POST /" + fixed + '/';
		std::string host_validator = " HTTP/1.1\r\nHost: " + host + "\r\n";
		bool isInputOk = false;

		unsigned int startIndex;
		unsigned int endIndex;

		unsigned int tempIndex;

		//If there's been a match at the beggining of the request...
		if (!(startIndex = message.find(validator_GET)))
			state = Connections::GET;
		else if (!(startIndex = message.find(validator_POST)))
			state = Connections::POST;
		else {
			state = Connections::NONE;
		}

		if (state != Connections::NONE) {
			if ((endIndex = message.find(host_validator)) == std::string::npos)
				state = Connections::NONE;

			else {
				//If the request has CRLF at end, then input was valid.
				if (message.length() >= (validator_GET.length() + host_validator.length())
					&& message.substr(message.length() - 2, 2) == "\r\n")

					isInputOk = true;
			}
		}
		else
			std::cout << "Client sent wrong input.\n";

		//Generates response (according to validity of input).

		if (isInputOk)
			answer(socket, index, message.substr(startIndex, endIndex - startIndex));
	}

	//If there's been an error, prints the message.
	else
		std::cout << error.message() << std::endl;
}

/*Called when there's been a connection.*/
void Server::connectionCallback(boost::asio::ip::tcp::socket& socket, unsigned int index, const boost::system::error_code& error) {
	if (!error) {
		//Sets socket to read request.
		socket.async_read_some
		(
			boost::asio::buffer(mess[index], MAXSIZE),
			boost::bind
			(
				&Server::inputValidation,
				this, boost::ref(socket), index, boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}

	else
		std::cout << error.message() << std::endl;
}
/*Called when a message has been sent to client.*/
void Server::messageCallback(const boost::system::error_code& error, size_t bytes_sent)
{
	//asyncConnection();
}

/*Responds to input.*/
void Server::answer(boost::asio::ip::tcp::socket& socket, unsigned int index, const std::string& message) {
	/*Generates text response, according to validity of input.*/
	switch (state) {
	case Connections::GET:
		responses[index] = GETResponse(message);
		if (!responses[index].length()) errorResponse();
		break;
	case Connections::POST:
		responses[index] = POSTResponse(message);
		if (!responses[index].length()) errorResponse();
		break;
	case Connections::NONE:
		errorResponse();
		break;
	default:
		break;
	}

	responses[index] += "\r\n\r\n";

	/*Sets socket to write (send to client).*/
	socket.async_write_some
	(
		boost::asio::buffer(responses[index]),
		boost::bind
		(
			&Server::messageCallback,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);

	/*Closes socket*/
	closeConnection(socket, index);

	asyncConnection(socket, index);
}

///*Generates http response, according to validity of input.*/
//void Server::GETResponse(bool isInputOk) {
//	/*if (isInputOk) {
//		response =
//			"HTTP/1.1 200 OK\r\nDate:" + date + "Location: " + TOT + "\r\nCache-Control: max-age=30\r\nExpires:" +
//			datePlusThirty + "Content-Length:" + std::to_string(size) +
//			"\r\nContent-Type: " + TYPE + "; charset=iso-8859-1\r\n\r\n";
//	}
//	else {
//		response =
//			"HTTP/1.1 404 Not Found\r\nDate:" + date + "Location: " + TOT +
//			"\r\nCache-Control: public, max-age=30 \r\nExpires:" + datePlusThirty +
//			"Content-Length: 0" + " \r\nContent-Type: " + TYPE + "; charset=iso-8859-1\r\n\r\n";
//	}*/
//}
//
//void Server::POSTResponse(bool isInputOk) {
//
//}
//
//void Server::errorReponse() {
//
//}

void Server::errorResponse(void) {
}