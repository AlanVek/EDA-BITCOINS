#pragma once

#include <allegro5/allegro.h>
#include "Filesystem/Filesystem.h"

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
/********************************/
const enum class NodeTypes {
	NEW_SVP,
	NEW_FULL,
};

class GUI {
private:

	struct NewNode {
		NewNode(const NodeTypes type, const unsigned int index) : type(type), index(index) { port = 0; ip.clear(); }
		NodeTypes type;
		std::string ip;
		int port;
		unsigned int index;
		std::vector<NewNode*> neighbors;
	};

public:

	GUI();

	~GUI();

	Events checkStatus(void);

	const std::vector<NewNode>& getNodes();

	bool nodeSelectionScreen(void);

private:
	const enum class States {
		INIT = 0,
		NODE_SELECTION,
		NODE_CONNECTION,
		NODE_CREATION,
		INIT_DONE,
		SENDER_SELECTION,
		RECEIVER_SELECTION,
		MESSAGE_SELECTION
	};

	/*Initial setup.*/
	/**********************************/
	void setAllegro();
	void initialImGuiSetup(void) const;
	/**********************************/

	/*Window displayers.*/
	/*************************************************************************************************/
	inline void newWindow() const;

	template <class Widget, class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const Widget&, const F1& f1, const F2 & = []() {}) -> decltype(f1());

	template <class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const char*, const F1& f1, const F2 & = []() {})->decltype(f1());

	inline void render() const;
	//inline void setAllFalse(const States&, bool = false);
	/*************************************************************************************************/

	/*Node creation and connection*/
	/***********************/
	void newNode(void);
	void connections(void);
	void creation(void);
	void showConnections(void);
	/***********************/

	/*Messages*/
	/**********************/
	void selectSender();
	void selectReceiver();
	void selectMessage();
	/**********************/

	/*Exit and resize events.*/
	bool eventManager(void);

	/*Allegro data members.*/
	/******************************/
	ALLEGRO_DISPLAY* guiDisp;
	ALLEGRO_EVENT_QUEUE* guiQueue;
	ALLEGRO_EVENT guiEvent;
	/******************************/

	/*Flag data members.*/
	/******************************/
	Events action;
	States state;
	/******************************/

	/*Data members modifiable by user.*/
	/**********************************/
	std::vector <NewNode> nodes;
	const NewNode* sender, * receiver;
	/**********************************/
};
