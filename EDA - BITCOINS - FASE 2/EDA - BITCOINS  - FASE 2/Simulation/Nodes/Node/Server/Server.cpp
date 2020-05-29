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
Server::Server(boost::asio::io_context& io_context_, const std::string& host) :
	host(host), io_context(io_context_), acceptor(io_context_,
		tcp::endpoint(tcp::v4(), 80)), socket(io_context_)
{
	if (socket.is_open()) {
		socket.shutdown(tcp::socket::shutdown_both);
		socket.close();
	}
	asyncConnection();
}

//Destructor. Closes open socket and acceptor.
Server::~Server() {
	std::cout << "\nClosing server.\n";
	if (socket.is_open()) {
		socket.shutdown(tcp::socket::shutdown_both);
		socket.close();
	}

	if (acceptor.is_open())
		acceptor.close();

	std::cout << "Server is closed.\n";
}

/*Sets acceptor to accept (asynchronously).*/
void Server::asyncConnection() {
	state = Connections::NONE;
	if (socket.is_open()) {
		//std::cout << "Error: Can't accept new connection from an open socket" << std::endl;
		return;
	}
	if (acceptor.is_open()) {
		std::cout << "Waiting for connection.\n";
		acceptor.async_accept(socket, boost::bind(&Server::connectionCallback, this, boost::asio::placeholders::error));
	}
	response.clear();
}

//Closes socket and clears message holder.
void Server::closeConnection() {
	socket.shutdown(tcp::socket::shutdown_both);
	socket.close();
	int i = 0;
	while (mess[i] != NULL) {
		mess[i] = NULL;
		i++;
	}
}

/*Validates input given in GET request.*/
void Server::inputValidation(const boost::system::error_code& error, size_t bytes) {
	if (!error) {
		//Creates string message from request.
		std::string message(mess);

		//Validator has the http protocol form.
		std::string validator_GET = "GET /" + fixed + '/';
		std::string validator_POST = "POST /" + fixed + '/';
		std::string host_validator = " HTTP/1.1\r\nHost: " + host + "\r\n";
		bool isInputOk = false;

		//If there's been a match at the beggining of the request...
		if (!message.find(validator_GET))
			state = Connections::GET;
		else if (!message.find(validator_POST))
			state = Connections::POST;
		else
			state = Connections::NONE;

		if (state != Connections::NONE) {
			if (message.find(host_validator) == std::string::npos)
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
		answer(isInputOk);
	}

	//If there's been an error, prints the message.
	else
		std::cout << error.message() << std::endl;
}

/*Called when there's been a connection.*/
void Server::connectionCallback(const boost::system::error_code& error) {
	if (!error) {
		//Sets socket to read request.
		socket.async_read_some
		(
			boost::asio::buffer(mess, MAXSIZE),
			boost::bind
			(
				&Server::inputValidation,
				this, boost::asio::placeholders::error,
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
	/*if (!error)
		std::cout << "Response sent correctly.\n\n";

	else
		std::cout << "Failed to respond to connection\n\n";*/

		/*Once the answer was sent, it frees acceptor for a new connection.*/
	asyncConnection();
}

/*Responds to input.*/
void Server::answer(bool isInputOk) {
	/*Generates text response, according to validity of input.*/
	switch (state) {
	case Connections::GET:
		GETResponse(isInputOk);
		break;
	case Connections::POST:
		POSTResponse(isInputOk);
		break;
	case Connections::NONE:
		errorResponse();
		break;
	default:
		break;
	}

	response += "\r\n\r\n";

	/*Sets socket to write (send to client).*/
	socket.async_write_some
	(
		boost::asio::buffer(response),
		boost::bind
		(
			&Server::messageCallback,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);

	/*Closes socket*/
	closeConnection();
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