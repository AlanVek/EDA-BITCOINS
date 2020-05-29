#pragma once

#include <allegro5/allegro.h>
#include "Filesystem/Filesystem.h"

/*GUI event codes.*/
/********************************/
const enum class Events : unsigned int {
	NOTHING = 0,
	END,
	SEE_MROOT,
	VALIDATE_MROOT,
	ALL_MERKLE,
	BLOCKID,
	PREVIOUS_BLOCKID,
	NTX,
	BLOCK_NUMBER,
	NONCE,
	NEW_FILE,
	PRINT_TREE,
	LOADED
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

	const unsigned int getBlockIndex() const;

	const std::string& getFilename(void);

	void setInfoShower(const std::string&);

	void setChainLength(unsigned int);
	void actionSolved(void);

private:
	const enum class States {
		INIT = 0,
		NODE_SELECTION,
		NODE_CONNECTION,
		NODE_CREATION,
		WAITING,
		FILE_OK,
		BLOCK_OK
	};

	/*Initial setup.*/
	/**********************************/
	void setAllegro();
	void initialImGuiSetup(void) const;
	/**********************************/

	/*Window displayers.*/
	/*************************************************************************************************/
	inline void newWindow() const;
	inline void displayPath();
	inline void displayActions();
	//bool displayFiles();
	void displayBlocks();

	template <class Widget, class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const Widget&, const F1& f1, const F2 & = []() {}) -> decltype(f1());

	template <class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const char*, const F1& f1, const F2 & = []() {})->decltype(f1());

	inline void render() const;
	inline void setAllFalse(const States&, bool = false);
	/*************************************************************************************************/

	/*Info update*/
	void newNode(void);
	void connections(void);
	void creation(void);

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
	bool force;
	unsigned int chainLength;
	std::string action_msg, shower;
	Events action;
	States state;
	/******************************/

	/*Data members modifiable by user.*/
	/**********************************/
	std::string path, selected;
	unsigned int index;
	std::vector <NewNode> nodes;
	/**********************************/

	/*File handling.*/
	/*************************************************************/
	Filesystem fs;
	const std::vector<std::string>& updateFiles(const char* = nullptr);
	/*************************************************************/
};
