#pragma once
#include <allegro5/allegro.h>
#include <vector>
#include <string>

class Node;

/*GUI event codes.*/
/********************************/
const enum class Events : unsigned int {
	NOTHING = 0,
	END,
	FILTER,
	MERKLEBLOCK,
	GET_BLOCKS,
	GET_HEADERS,
	TRANSACTION,
	POST_BLOCK
};

const enum class Shower : int {
	NOTHING = 0,
	SEE_MROOT,
	VALIDATE_MROOT,
	BLOCKID,
	PREVIOUS_BLOCKID,
	NTX,
	BLOCK_NUMBER,
	NONCE,
	PRINT_TREE,
	MODIFY_NEIGHBOURS
};
/********************************/
const enum class NodeTypes {
	NEW_SPV,
	NEW_FULL,
	UNDEFINED
};

class GUI {
private:

	/*Node struct to keep node info in GUI.*/
	struct NewNode {
		/*Constructor.*/
		NewNode(const NodeTypes type, const unsigned int index) : type(type), index(index) { port = 0; ip.clear(); }
		NewNode() {}

		/*Data*/
		/*********************************/
		NodeTypes type;
		std::string ip;
		int port;
		unsigned int index;
		std::vector<unsigned int> neighbors;
		/*********************************/
	};

public:

	GUI();

	~GUI();

	/*General screen.*/
	Events checkStatus(void);

	/*Main screen.*/
	bool nodeSelectionScreen(void);

	/*Getters.*/
	const std::vector<NewNode>& getNodes();
	const unsigned int& getSenderID();
	const unsigned int& getReceiverID();
	const NewNode& getNode(unsigned int index);

	const int getAmount();
	const std::string& getWallet();

	void updateMsg(const std::string&);

	void infoGotten();

	void setRealNodes(const std::vector < Node*>&);

private:

	/*GUI states.*/
	/****************************/
	const enum class States {
		INIT = 0,
		GENESIS_MODE,
		APPENDIX_MODE,
		NODE_SELECTION,
		NODE_CONNECTION,
		NODE_CREATION,
		INIT_DONE,
		SENDER_SELECTION,
		RECEIVER_SELECTION,
		MESSAGE_SELECTION,
		PARAM_SELECTION
	};
	/****************************/

	/*Initial setup.*/
	/**********************************/
	void setAllegro();
	void initialImGuiSetup(void) const;
	/**********************************/

	/*Window displayers.*/
	/*************************************************************************************************/
	inline void newWindow(const char*) const;

	template <class Widget, class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const Widget&, const F1& f1, const F2 & = []() {}) -> decltype(f1());

	template <class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const char*, const F1& f1, const F2 & = []() {})->decltype(f1());

	inline void render() const;
	/*************************************************************************************************/

	/*Node creation and connection*/
	/***********************/
	bool init(bool*);
	void newNode(void);
	void connections(void);
	void creation(void);
	void showConnections(void);
	void setConnectionStr(void);
	void showNetworkingInfo(void);
	void genesisConnection(void);
	void addNeighbour(bool);
	/***********************/

	/*Messages*/
	/**********************/
	void selectSender();
	void selectReceiver();
	void selectMessage();
	void selectParameters();
	/**********************/

	/*Exit and resize events.*/
	bool eventManager(void);
	void generalScreen(void);

	/*Allegro data members.*/
	/******************************/
	ALLEGRO_DISPLAY* guiDisp;
	ALLEGRO_EVENT_QUEUE* guiQueue;
	ALLEGRO_EVENT guiEvent;
	/******************************/

	/*Displaying nodes*/
	void showBlocks();
	void displayActions();
	void showNodes();
	void displaySPVActions();

	/*Flag data members.*/
	/******************************/
	Events action;
	States state;
	/******************************/

	/*Data members modifiable by user.*/
	/**********************************/
	std::vector <NewNode> nodes;
	std::vector <Node*> allNodes;
	unsigned int sender, receiver;
	std::string wallet;
	NewNode newNeighbor;
	int amount;
	std::string networkingInfo, nodeConnections;
	bool showingConnections, showingNetworking, addingNeighbor, isLocal, selected;
	int currentIndex, dataIndex;
	Shower showingBlock;
	unsigned int ID;
	/**********************************/
};
