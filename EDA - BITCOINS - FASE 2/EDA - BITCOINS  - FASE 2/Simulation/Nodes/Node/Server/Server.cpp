#include "Server.h"
#include <iostream>
#include <boost\bind.hpp>
#include <fstream>
#include "json.hpp"

using boost::asio::ip::tcp;

namespace {
	const std::string fixed = "eda_coins";
}

/*Server constructor. Initializes io_context, acceptor and socket.
Calls asyncConnection to accept connections.*/
Server::Server(boost::asio::io_context& io_context_, const std::string& host, const Response& GET,
	const Response& POST, unsigned int port) : host(host), port(port), io_context(io_context_),
	acceptor(io_context_, tcp::endpoint(tcp::v4(), port))
{
	/*Sets GET and POST callbacks.*/
	GETResponse = GET;
	POSTResponse = POST;

	newConnector();
}

/*If there's a new neighbor, it sets a new socket.*/
void Server::newConnector() {
	/*Pushes a new socket.*/

	sockets.push_back(Connection(io_context));
	sockets.back().pos = std::prev(sockets.end());

	/*Closes socket (in case it's open).*/
	if (sockets.back().socket.is_open()) {
		sockets.back().socket.shutdown(tcp::socket::shutdown_both);
		sockets.back().socket.close();
	}
	for (unsigned int i = 0; i < MAXSIZE; i++)
		sockets.back().reader[i] = NULL;

	/*Connects socket with acceptor.*/
	asyncConnection(sockets.back());
}

//Destructor. Closes open sockets and acceptor.
Server::~Server() {
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
		if (!(connector).socket.is_open()) {
			acceptor.async_accept(
				(connector).socket, boost::bind(&Server::connectionCallback,
					this, boost::ref(connector), boost::asio::placeholders::error)
			);
		}
	}
}

//Closes socket and clears message holder.
void Server::closeConnection(iterator connector) {
	(connector).socket.shutdown(tcp::socket::shutdown_both);
	(connector).socket.close();

	/*If there are more sockets, it erases the current one.*/
	if (sockets.size() > 1)
		sockets.erase(connector.pos);

	/*Otherwise...*/
	else {
		/*It clears the reader and reconnects.*/
		for (unsigned int i = 0; i < MAXSIZE; i++)
			(connector).reader[i] = NULL;

		asyncConnection(connector);
	}
}

/*Validates input given in GET request.*/
void Server::inputValidation(iterator connector, const boost::system::error_code& error, size_t bytes) {
	if (!error) {
		std::string message = (connector).reader;
		/*	if (message.find("POST") != std::string::npos{*/
		if (message.find("POST") != std::string::npos) {
			if (message.find("Content-Length=") == std::string::npos)
				answer(connector, message);
			else {
				int size, pos;
				std::string temp;
				pos = message.find("Content-Length=");
				while (message[pos + 15] != '\r') {
					temp.append(1, message[pos + 15]);
					pos++;
				}
				if (message.substr(message.find("Data=") + 5, message.length()).length() < std::stoi(temp))
					return;
			}
		}

		//Validator has the http protocol form.
		std::string validator_GET = "GET /" + fixed + '/';
		std::string validator_POST = "POST /" + fixed + '/';
		std::string host_validator = " HTTP/1.1\r\nHost: " + host + ':' + std::to_string(port);

		/*Sets indexes to cut message.*/
		int startIndex = 0;
		int endIndex = message.length();

		//If it's a GET request, sets state to GET.
		if (!(startIndex = message.find(validator_GET)))
			state = Connections::GET;

		//If it's a POST request, sets state to POST.
		else if (!(startIndex = message.find(validator_POST)))
			state = Connections::POST;

		/*Otherwise, it's an error.*/
		else {
			state = Connections::NONE;
		}

		/*If input was ok...*/
		if (state != Connections::NONE) {
			/*Checks for error in the host header.*/
			if ((endIndex = message.find(host_validator)) == std::string::npos)
				state = Connections::NONE;
			else if (state == Connections::POST) {
				if (message.find("Data") != std::string::npos) {
					endIndex = message.length();
				}
				else
					state = Connections::NONE;
			}
		}
		else
			std::cout << "Client sent wrong input.\n";

		//Answers request.
		answer(connector, message.substr(startIndex >= 0 ? startIndex : 0, endIndex >= 0 ? endIndex : message.length()));
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
		connector.socket.async_read_some
		(
			boost::asio::buffer((connector).reader, MAXSIZE),
			boost::bind
			(
				&Server::inputValidation,
				this, boost::ref(connector), boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}

	else
		std::cout << error.message() << std::endl;
}
/*Called when a message has been sent to client.*/
void Server::messageCallback(iterator connector, const boost::system::error_code& error, size_t bytes_sent)
{
	if (error)
		std::cout << error.message() << std::endl;
	else
		std::cout << "Answered." << std::endl;

	/*Closes socket*/
	closeConnection(connector);
}

/*Responds to input.*/
void Server::answer(iterator connector, const std::string& message) {
	/*Generates text response, according to validity of input.*/
	switch (state) {
		/*GET request. Calls GET callback.*/
	case Connections::GET:
		(connector).response = GETResponse(message);
		break;

		/*POST request. Calls POST callback.*/
	case Connections::POST:
		(connector).response = POSTResponse(message);
		break;

		/*Error. Calls error callback.*/
	case Connections::NONE:
		(connector).response = errorResponse();
		break;
	default:
		break;
	}

	(connector).response += "\r\n\r\n";

	/*Sets socket to write (send to client).*/
	(connector).socket.async_write_some
	(
		boost::asio::buffer((connector).response),
		boost::bind
		(
			&Server::messageCallback,
			this, boost::ref(connector),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

/*Generates http response, according to validity of input.*/
const std::string Server::errorResponse(void) {
	nlohmann::json temp;

	temp["status"] = "false";
	temp["result"] = 1;

	return temp.dump();
}