#include "Server.h"
#include <iostream>
#include <boost\bind.hpp>
#include "json.hpp"

using boost::asio::ip::tcp;

namespace {
	const std::string fixed = "eda_coins";
	const char* autoIP = "127.0.0.1";
}

/*Server constructor. Initializes io_context, acceptor and socket.
Calls asyncConnection to accept connections.*/
Server::Server(boost::asio::io_context& io_context_, const Response& GET,
	const Response& POST, const errorResp& ERROR_RESP, unsigned int port) : host(autoIP), port(port), io_context(io_context_),
	acceptor(io_context_, tcp::endpoint(tcp::v4(), port)), GETResponse(GET), POSTResponse(POST), errorResponse(ERROR_RESP)
{
	newConnector();
}

/*If there's a new neighbor, it sets a new socket.*/
void Server::newConnector() {
	/*Pushes a new socket.*/
	sockets.push_back(Connection(io_context));

	/*Closes socket (in case it's open).*/
	if (sockets.back().socket.is_open()) {
		sockets.back().socket.shutdown(tcp::socket::shutdown_both);
		sockets.back().socket.close();
	}
	for (unsigned int i = 0; i < MAXSIZE; i++)
		sockets.back().reader[i] = NULL;

	/*Connects socket with acceptor.*/
	asyncConnection(&sockets.back());
}

//Destructor. Closes open sockets and acceptor.
Server::~Server() {
	/*Closes open sockets.*/
	for (auto& connector : sockets) {
		if (connector.socket.is_open()) {
			connector.socket.shutdown(tcp::socket::shutdown_both);
			connector.socket.close();
		}
	}

	/*Closes acceptor.*/
	if (acceptor.is_open())
		acceptor.close();
}

/*Sets acceptor to accept (asynchronously) with specific socket.*/
void Server::asyncConnection(Connection* connector) {
	if (acceptor.is_open()) {
		if (!connector->socket.is_open()) {
			acceptor.async_accept(
				connector->socket, boost::bind(&Server::connectionCallback,
					this, connector, boost::asio::placeholders::error)
			);
		}
	}
}

//Closes socket and clears message holder.
void Server::closeConnection(Connection* connector) {
	connector->socket.shutdown(tcp::socket::shutdown_both);
	connector->socket.close();

	/*If there are more sockets, it erases the current one.*/
	if (sockets.size() > 1) {
		auto ptr = sockets.begin();
		for (auto i = sockets.begin(); i != sockets.end(); i++)
			if (&(*i) == connector)
				ptr = i;
		sockets.erase(ptr);
		newConnector();
	}

	/*Otherwise...*/
	else {
		/*It clears the reader and reconnects.*/
		for (unsigned int i = 0; i < MAXSIZE; i++)
			connector->reader[i] = NULL;

		asyncConnection(connector);
	}
}

/*Validates input given in GET request.*/
void Server::inputValidation(Connection* connector, const boost::system::error_code& error, size_t bytes) {
	if (!error) {
		std::string message = (*connector).reader;

		//Validator has the http protocol form.
		std::string validator_GET = "GET /" + fixed + '/';
		std::string validator_POST = "POST /" + fixed + '/';
		std::string host_validator = " HTTP/1.1\r\nHost: " + host /*+ ':' + std::to_string(port)*/;

		//If it's a GET request, sets type to GET.
		if (!message.find(validator_GET))
			type = ConnectionTypes::GET;

		//If it's a POST request, sets type to POST.
		else if (!message.find(validator_POST))
			type = ConnectionTypes::POST;

		/*Otherwise, it's an error.*/
		else {
			type = ConnectionTypes::NONE;
		}

		if (message.find(host_validator) == std::string::npos)
			type = ConnectionTypes::NONE;

		//Answers request.
		answer(connector, message);
	}

	//If there's been an error, prints the message.
	else
		std::cout << "Failed to respond. Error: " << error.message() << std::endl;
}

/*Called when there's been a connection.*/
void Server::connectionCallback(Connection* connector, const boost::system::error_code& error) {
	newConnector();

	connector->state = ServerState::PERFORMING;

	if (!error) {
		//Sets socket to read request.
		connector->socket.async_read_some(
			//connector.socket,
			boost::asio::buffer(connector->reader, MAXSIZE),
			//boost::asio::transfer_all(),
			boost::bind
			(
				&Server::inputValidation,
				this, connector, boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}

	else
		std::cout << "Failed to respond. Error: " << error.message() << std::endl;
}
/*Called when a message has been sent to client.*/
void Server::messageCallback(Connection* connector, const boost::system::error_code& error, size_t bytes_sent)
{
	if (error)
		std::cout << "Failed to respond. Error: " << error.message() << std::endl;
	/*else
		std::cout << "Answered." << std::endl;*/

		/*Closes socket*/
	closeConnection(connector);

	connector->state = ServerState::FINISHED;
}

/*Responds to input.*/
void Server::answer(Connection* connector, const std::string& message) {
	/*Generates text response, according to validity of input.*/
	switch (type) {
		/*GET request. Calls GET callback.*/
	case ConnectionTypes::GET:
		(connector)->response = GETResponse(message, connector->socket.remote_endpoint());
		break;

		/*POST request. Calls POST callback.*/
	case ConnectionTypes::POST:
		(connector)->response = POSTResponse(message, connector->socket.remote_endpoint());
		break;

		/*Error. Calls error callback.*/
	case ConnectionTypes::NONE:
		(connector)->response = errorResponse();
		break;
	default:
		break;
	}

	(connector)->response += "\r\n\r\n";

	/*Sets socket to write (send to client).*/
	(connector)->socket.async_write_some
	(
		boost::asio::buffer(connector->response),
		boost::bind
		(
			&Server::messageCallback,
			this, connector,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

/*Returns vector with sockets connection states.*/
std::vector<stateOfConnection> Server::getState() {
	std::vector<stateOfConnection> temp;
	boost::asio::ip::tcp::endpoint connection;

	/*Sets new vector with sockets state.*/
	for (auto& socket : sockets) {
		switch (socket.state) {
		case ServerState::FREE:
			temp.push_back(stateOfConnection(socket.state, "", 0));
			break;
		case ServerState::PERFORMING:
			connection = socket.socket.remote_endpoint();
			temp.push_back(stateOfConnection(socket.state, connection.address().to_string(), connection.port()));
			break;
		case ServerState::FINISHED:
			temp.push_back(stateOfConnection(socket.state, "", 0));
			socket.state = ServerState::FREE;
			break;
		default:
			temp.push_back(stateOfConnection(socket.state, "", 0));
			break;
		}
	}
	return temp;
}