#pragma once

#include <allegro5/allegro.h>
#include "Filesystem/Filesystem.h"

/*GUI event codes.*/
/********************************/
const enum class Events : int {
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
	PRINT_TREE
};
/********************************/

const enum class States {
	INIT = 0,
	WAITING,
	FILE_OK,
	BLOCK_OK
};

class GUI {
public:

	GUI();

	~GUI();

	const Events checkStatus(void);

	const unsigned int getBlockIndex() const;

	const std::string& getFilename(void);

	void setInfoShower(const std::string&);

	void setChainLength(unsigned int);
	void actionSolved(void);

private:
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
	bool displayFiles();
	void displayBlocks();

	template <class Widget, class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const Widget&, const F1& f1, const F2 & = []() {}) -> decltype(f1());

	template <class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const char*, const F1& f1, const F2 & = []() {})->decltype(f1());

	inline void render() const;
	inline void setAllFalse(const States&, bool = false);
	/*************************************************************************************************/

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
	/**********************************/

	/*File handling.*/
	/*************************************************************/
	Filesystem fs;
	const std::vector<std::string>& updateFiles(const char* = nullptr);
	/*************************************************************/
};
