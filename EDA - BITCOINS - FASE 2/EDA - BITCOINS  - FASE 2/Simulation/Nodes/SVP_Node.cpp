#include "SVP_Node.h"

SVP_Node::SVP_Node(boost::asio::io_context& io_context) : Node(io_context) {}

SVP_Node::~SVP_Node() {}

void SVP_Node::connect(const std::string& ip, const unsigned int) {}

void SVP_Node::newNeighbor(const std::string& ip, const unsigned int port) { neighbors.push_back(Neighbor(ip, port)); }