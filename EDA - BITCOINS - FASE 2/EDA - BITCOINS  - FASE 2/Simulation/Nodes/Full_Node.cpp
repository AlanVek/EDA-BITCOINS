#include "Full_Node.h"

Full_Node::Full_Node(boost::asio::io_context& io_context) : Node(io_context) {}

Full_Node::~Full_Node() {}

void Full_Node::connect(const std::string& ip, const unsigned int) {}
void Full_Node::connectionCallback() {}

void Full_Node::newNeighbor(const std::string& ip, const unsigned int port) { neighbors.push_back(Neighbor(ip, port)); }