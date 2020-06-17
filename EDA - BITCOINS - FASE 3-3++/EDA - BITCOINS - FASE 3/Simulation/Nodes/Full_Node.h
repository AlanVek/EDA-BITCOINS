#pragma once
#include "Node/Node.h"
#include "BlockChain/BlockChain.h"
#include <allegro5/allegro.h>

/*****Full node events********/

const enum class NodeEvents : unsigned int {
	NOTHING = 0,
	PING,
	TIMEOUT,
	NETWORK_NOT_READY,
	NETWORK_READY,
	NETWORK_LAYOUT,
	SEND_NOT_READY,
	ADD_NEIGHBORS,
	SEND_READY,
	END
};
/*****Full node states********/

const enum class NodeState : unsigned int {
	IDLE = 0,
	WAITING_LAYOUT,
	COLLECTING_MEMBERS,
	NETWORK_CREATED,
	END
};

class Full_Node : public Node {
public:
	Full_Node(boost::asio::io_context&, const std::string&, const unsigned int, const unsigned int, int&,
		const GUIMsg&);
	virtual ~Full_Node();

	virtual void perform(ConnectionType, const unsigned int, const std::string&, const unsigned int);
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const std::string&);
	virtual void perform();

	virtual const json& getData() { return blockChain.getRawData(); }

	virtual bool networkDone() { return state == NodeState::NETWORK_CREATED; }

	virtual const std::string printTree(unsigned int idx) { return blockChain.getBlockInfo(idx, BlockInfo::PRINT_TREE); }
	virtual const std::string validateMRoot(unsigned int idx) { return blockChain.getBlockInfo(idx, BlockInfo::VALIDATE_MROOT); }

	virtual const std::string& getKey() { return std::string(); }

	virtual void startTimer() { if (timer) al_start_timer(timer); }

	virtual void checkTimeout(const std::vector < Node*>&);

	virtual std::vector<Neighbor> getAdders() { auto temp = idsToAdd; idsToAdd.clear(); return idsToAdd; }

protected:

	std::string dispatcher(NodeEvents, const boost::asio::ip::tcp::endpoint& nodeInfo, const std::string&);		/**************************************/

	const json getMerkleBlock(const std::string&, const std::string&);
	const json& getBlock(const std::string& blockID);

	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&);

	void particularAlgorithm();

	void neighborFromJSON(const std::string&, bool);
	void setIndexes(std::map<int, std::string>&, json&, int);
	std::vector<Neighbor> idsToAdd;

	std::vector<Node*> subNet;
	BlockChain blockChain;
	NodeState state;
	NodeEvents Event;

	ALLEGRO_TIMER* timer;
	ALLEGRO_EVENT_QUEUE* queue;
	ALLEGRO_EVENT timeEvent;

	json transactions;

	bool validateTransaction(const json&, bool);
	bool validateBlock(const json&);

	const json generateTransJSON(const std::string&, const unsigned int);

	void updateUTXOs(const json&);

	int pingSent;
};
