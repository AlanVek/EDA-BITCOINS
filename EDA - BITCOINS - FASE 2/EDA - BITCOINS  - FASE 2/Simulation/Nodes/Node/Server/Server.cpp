#include "Server.h"

#include <iostream>
#include <boost\bind.hpp>
#include <chrono>
#include <fstream>

using boost::asio::ip::tcp;

#define HOST (std::string) "127.0.0.1"
#define PATH (std::string) "img"
#define FILENAME (std::string) "page/page.html"
#define TYPE "text/html"

#define TOT (HOST+'/'+PATH+'/'+FILENAME)

std::string makeDaytimeString(bool plusThirty);

/*Server constructor. Initializes io_context, acceptor and socket.
Calls waitForConnection to accept connections.*/
Server::Server(boost::asio::io_context& io_context_) :
	io_context(io_context_), acceptor(io_context_, tcp::endpoint(tcp::v4(), 80)), socket(io_context_)
{
	/*if (socket.is_open()) {
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
	}
	waitForConnection();*/
}

//Destructor. Closes open socket and acceptor.
Server::~Server() {
	//std::cout << "\nClosing server.\n";
	if (socket.is_open()) {
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
	}

	if (acceptor.is_open())
		acceptor.close();

	//std::cout << "Server is closed.\n";
}

/*Sets acceptor to accept (asynchronously).*/
void Server::waitForConnection() {
	if (socket.is_open()) {
		std::cout << "Error: Can't accept new connection from an open socket" << std::endl;
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
	socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
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
		std::string validator = "GET /" + PATH + '/' + FILENAME + " HTTP/1.1\r\nHost: " + HOST + "\r\n";
		bool isInputOk = false;

		//If there's been a match at the beggining of the request...
		if (message.find(validator) == 0) {
			//If the request has CRLF at end, then input was valid.
			if (message.length() > validator.length()
				&& message[message.length() - 2] == '\r'
				&& message[message.length() - 1] == '\n')

				isInputOk = true;
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
	if (!error)
		std::cout << "Response sent correctly.\n\n";

	else
		std::cout << "Failed to respond to connection\n\n";

	/*Once the answer was sent, it frees acceptor for a new connection.*/
	waitForConnection();
}

/*Returns daytime string. If plusThirty is true, it returns
current daytime + 30 seconds.*/
std::string makeDaytimeString(bool plusThirty) {
	using namespace std::chrono;
	system_clock::time_point theTime = system_clock::now();

	if (plusThirty)
		theTime += seconds(30);

	time_t now = system_clock::to_time_t(theTime);

	return ctime(&now);
}

/*Responds to input.*/
void Server::answer(bool isInputOk) {
	//Opens file.
	std::fstream page(FILENAME, std::ios::in | std::ios::binary);

	/*Checks if file was correctly open.*/
	if (!page.is_open()) {
		std::cout << "Failed to open file\n";
		return;
	}

	size = getFileSize(page);

	/*Generates text response, according to validity of input.*/
	response = generateTextResponse(isInputOk);

	/*If input was correct, appends text from file.*/
	if (isInputOk) {
		std::ostringstream ss;
		ss << page.rdbuf();
		response += ss.str();
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

	//Closes file.
	page.close();
}

/*Generates http response, according to validity of input.*/
std::string Server::generateTextResponse(bool isInputOk) {
	std::string date = makeDaytimeString(false);
	std::string datePlusThirty = makeDaytimeString(true);
	std::string response;
	if (isInputOk) {
		response =
			"HTTP/1.1 200 OK\r\nDate:" + date + "Location: " + TOT + "\r\nCache-Control: max-age=30\r\nExpires:" +
			datePlusThirty + "Content-Length:" + std::to_string(size) +
			"\r\nContent-Type: " + TYPE + "; charset=iso-8859-1\r\n\r\n";
	}
	else {
		response =
			"HTTP/1.1 404 Not Found\r\nDate:" + date + "Location: " + TOT +
			"\r\nCache-Control: public, max-age=30 \r\nExpires:" + datePlusThirty +
			"Content-Length: 0" + " \r\nContent-Type: " + TYPE + "; charset=iso-8859-1\r\n\r\n";
	}

	return response;
}
/*Gets file length.*/
size_t Server::getFileSize(std::fstream& file) {
	file.seekg(0, file.end);

	size_t length = file.tellg();

	file.seekg(0, file.beg);
	return length;
}