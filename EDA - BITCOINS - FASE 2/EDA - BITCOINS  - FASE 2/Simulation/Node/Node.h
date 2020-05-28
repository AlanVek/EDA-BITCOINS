#pragma once
#include "Client/Client.h"
#include "Server/Server.h"
class Node
{
public:
	Node();
private:
	Client client;
	Server server;
};
