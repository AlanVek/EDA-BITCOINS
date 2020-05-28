#pragma once
#include <array>
#include "eventHandling.h"
#include "GUI/GUI.h"

//	const enum : unsigned int {
//		NOTHING = 0,
//		END,
//		SEE_MROOT,
//		VALIDATE_MROOT,
//		ALL_MERKLE,
//		BLOCKID,
//		PREVIOUS_BLOCKID,
//		NTX,
//		BLOCK_NUMBER,
//		NONCE,
//		NEW_FILE,
//		PRINT_TREE
//	};
//}

class Nothing : public genericEvent
{
public:
	eventTypes getType(void) { return Events::NOTHING; }
};

class End : public genericEvent {
public:
	eventTypes getType(void) { return Events::END; }
};

class GUIEvents : public eventGenerator
{
public:
	GUIEvents();
	genericEvent* getEvent(void);
	~GUIEvents() {};

private:

	GUI gui;
};
