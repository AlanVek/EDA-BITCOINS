#include "Server.h"
#include <iostream>
#include <boost\bind.hpp>
#include <fstream>
#include "json.hpp"

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
	/*Sets GET and POST callbacks.*/
	GETResponse = GET;
	POSTResponse = POST;

	newConnector();
}

/*If there's a new neighbor, it sets a new socket.*/
void Server::newConnector() {
	/*Pushes a new socket.*/
	sockets.push_back({ boost::asio::ip::tcp::socket(io_context), std::string() });

	/*Closes socket (in case it's open).*/
	if (sockets.back().socket.is_open()) {
		sockets.back().socket.shutdown(tcp::socket::shutdown_both);
		sockets.back().socket.close();
	}

	/*Connects socket with acceptor.*/
	asyncConnection(std::prev(sockets.end()));
}

//Destructor. Closes open sockets and acceptor.
Server::~Server() {
	std::cout << "\nClosing server.\n";

	/*Closes open sockets.*/
	for (auto& socket : sockets) {
		if (socket.socket.is_open()) {
			socket.socket.shutdown(tcp::socket::shutdown_both);
			socket.socket.close();
		}
	}

	/*Closes acceptor.*/
	if (acceptor.is_open())
		acceptor.close();

	std::cout << "Server is closed.\n";
}

/*Sets acceptor to accept (asynchronously) with specific socket.*/
void Server::asyncConnection(iterator connector) {
	if (acceptor.is_open()) {
		std::cout << "Waiting for connection.\n";
		if (!(*connector).socket.is_open()) {
			acceptor.async_accept(
				(*connector).socket, boost::bind(&Server::connectionCallback,
					this, connector, boost::asio::placeholders::error)
			);
		}
		(*connector).response.clear();
	}
}

//Closes socket and clears message holder.
void Server::closeConnection(iterator connector) {
	(*connector).socket.shutdown(tcp::socket::shutdown_both);
	(*connector).socket.close();

	if (sockets.size() > 1)
		sockets.erase(connector);
}

/*Validates input given in GET request.*/
void Server::inputValidation(iterator connector, const boost::system::error_code& error, size_t bytes) {
	if (!error) {
		//Creates string message from request.
		//std::string message(mess[index]);

		//Validator has the http protocol form.
		std::string validator_GET = "GET /" + fixed + '/';
		std::string validator_POST = "POST /" + fixed + '/';
		std::string host_validator = " HTTP/1.1\r\nHost: " + host + "\r\n";

		/*Sets indexes to cut message.*/
		unsigned int startIndex = 0;
		unsigned int endIndex = (*connector).reader.length();

		//If it's a GET request, sets state to GET.
		if (!(startIndex = (*connector).reader.find(validator_GET)))
			state = Connections::GET;

		//If it's a POST request, sets state to POST.
		else if (!(startIndex = (*connector).reader.find(validator_POST)))
			state = Connections::POST;

		/*Otherwise, it's an error.*/
		else {
			state = Connections::NONE;
		}

		/*If input was ok...*/
		if (state != Connections::NONE) {
			/*Checks for error in the host header.*/
			if ((endIndex = (*connector).reader.find(host_validator)) == std::string::npos)
				state = Connections::NONE;

			else {
				//If the request has CRLF at end, then input was valid.
				if ((*connector).reader.length() >= (validator_GET.length() + host_validator.length())
					&& (*connector).reader.substr((*connector).reader.length() - 2, 2) == "\r\n")

					state = Connections::NONE;
			}
		}
		else
			std::cout << "Client sent wrong input.\n";

		//Answers request..
		answer(connector, (*connector).reader.substr(startIndex, endIndex - startIndex));
	}

	//If there's been an error, prints the message.
	else
		std::cout << error.message() << std::endl;
}

/*Called when there's been a connection.*/
void Server::connectionCallback(iterator connector, const boost::system::error_code& error) {
	newConnector();

	if (!error) {
		//Sets socket to read request.
		(*connector).socket.async_read_some
		(
			boost::asio::buffer((*connector).reader),
			boost::bind
			(
				&Server::inputValidation,
				this, connector, boost::asio::placeholders::error,
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
void Server::answer(iterator connector, const std::string& message) {
	/*Generates text response, according to validity of input.*/
	switch (state) {
		/*GET request. Calls GET callback.*/
	case Connections::GET:
		(*connector).response = GETResponse(message);
		if (!(*connector).response.length())
			break;

		/*POST request. Calls POST callback.*/
	case Connections::POST:
		(*connector).response = POSTResponse(message);
		if (!(*connector).response.length())
			break;

		/*Error. Calls error callback.*/
	case Connections::NONE:
		(*connector).response = errorResponse();
		break;
	default:
		break;
	}

	(*connector).response += "\r\n\r\n";

	/*Sets socket to write (send to client).*/
	(*connector).socket.async_write_some
	(
		boost::asio::buffer((*connector).response),
		boost::bind
		(
			&Server::messageCallback,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);

	/*Closes socket*/
	closeConnection(connector);

	/*Reconnects socket with acceptor.*/
	asyncConnection(connector);
}

/*Generates http response, according to validity of input.*/
const std::string Server::errorResponse(void) {
	nlohmann::json temp;

	temp["status"] = false;
	temp["result"] = 1;

	return temp.dump();
}