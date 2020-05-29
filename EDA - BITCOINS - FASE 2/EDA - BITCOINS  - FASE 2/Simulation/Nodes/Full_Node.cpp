#include "Full_Node.h"
#include "Node/Client/AllClients.h"
#include "Node/Server/Full_Server.h"

Full_Node::Full_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
	server = new Full_Server(io_context, ip);
}

void Full_Node::NEWGET(const unsigned int& id, const ConnectionType type, const std::string& blockID, const unsigned int count)
{
	if (state == States::FREE && !client) {
		if (neighbors.find(id) != neighbors.end() && count && type == ConnectionType::GETBLOCK) {
			client = new GETBlockClient(neighbors[id].ip, neighbors[id].port, blockID, count);
			state = States::CLIENTMODE;
		}
	}
}

void Full_Node::NEWPOST(const unsigned int& id, const ConnectionType type, const json& header) {
	if (!client && state == States::FREE && neighbors.find(id) != neighbors.end() && !header.is_null()) {
		state = States::CLIENTMODE;
		switch (type) {
		case ConnectionType::POSTBLOCK:
			client = new BlockClient(neighbors[id].ip, neighbors[id].port, header);
			break;
		case ConnectionType::POSTFILTER:
			client = new FilterClient(neighbors[id].ip, neighbors[id].port, header);
			break;
		case ConnectionType::POSTTRANS:
			client = new TransactionClient(neighbors[id].ip, neighbors[id].port, header);
			break;
		default:
			state = States::FREE;
			break;
		}
	}
}

void Full_Node::perform() {
	if (client && state == States::CLIENTMODE) {
		if (!client->perform()) {
			delete client;
			client = nullptr;
			state = States::FREE;
		}
	}
}

const unsigned int& Full_Node::getID() { return identifier; }
const Full_Node::States Full_Node::getState(void) { return state; }

Full_Node::~Full_Node() {
	if (client) {
		delete client;
		client = nullptr;
	}
	if (server) {
		delete server;
		server = nullptr;
	}
}

void Full_Node::newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port)
{
	neighbors[id] = { ip, port };
}