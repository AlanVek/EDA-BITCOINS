#pragma once
#include <string>
class Node;

class Action {
public:

	Action(Node* node, const std::string& name) : name(name), node(node) {};

	virtual ~Action(void) {};

	virtual void Perform(const unsigned int, const std::string&, const unsigned int) = 0;
	virtual void Perform(const unsigned int, const std::string&, const std::string&) = 0;

	const std::string& getName() { return name; }
	void setData(const json& data) { this->data = data; }

protected:
	const std::string name;
	Node* node;
	json data;
};

class POSTBlock : public Action {
public:
	POSTBlock(Node*);

	virtual void Perform(const unsigned int, const std::string&, const unsigned int = 0);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class POSTMerkle : public Action {
public:
	POSTMerkle(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int) {};
	virtual void Perform(const unsigned int, const std::string&, const std::string&);
};
class POSTTrans : public Action {
public:
	POSTTrans(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class POSTFilter : public Action {
public:
	POSTFilter(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class GETBlock : public Action {
public:
	GETBlock(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
class GETHeader : public Action {
public:
	GETHeader(Node*);
	virtual void Perform(const unsigned int, const std::string&, const unsigned int);
	virtual void Perform(const unsigned int, const std::string&, const std::string&) {};
};
