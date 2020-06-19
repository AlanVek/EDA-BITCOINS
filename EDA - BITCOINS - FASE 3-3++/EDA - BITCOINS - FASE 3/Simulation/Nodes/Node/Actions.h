#pragma once
#include "Node.h"

class POSTBlock : public Node::Action {
public:
	POSTBlock(Node*);

	virtual void Perform(const unsigned int, const std::string&, const unsigned int = 0);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class POSTMerkle : public Node::Action {
public:
	POSTMerkle(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int) {};
	virtual void Perform(const unsigned int, const std::string&, const std::string&);
};
class POSTTrans : public Node::Action {
public:
	POSTTrans(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class POSTFilter : public Node::Action {
public:
	POSTFilter(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class GETBlock : public Node::Action {
public:
	GETBlock(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class GETHeader : public Node::Action {
public:
	GETHeader(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class Ping : public Node::Action {
public:
	Ping(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class Layout : public Node::Action {
public:
	Layout(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class FalseTrans : public Node::Action {
public:
	FalseTrans(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};