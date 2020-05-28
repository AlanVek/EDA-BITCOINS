#include <iostream>
#include "Server.h"
#include <chrono>

using namespace std::chrono;

int main() {
	boost::asio::io_context io_context;

	//Creates instance of server.
	Server myNewServer(io_context);

	//Runs server for one minute.
	io_context.run_until(system_clock::now() + seconds(30));
}