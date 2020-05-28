#pragma once
#include "Simulation/Client/Client.h"
#include "Simulation/Server/Server.h"
class Node
{
public:
	Node();
private:
	Client client;
	Server server;
};
