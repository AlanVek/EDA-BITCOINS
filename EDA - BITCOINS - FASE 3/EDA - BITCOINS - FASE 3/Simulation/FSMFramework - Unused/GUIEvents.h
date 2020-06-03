#pragma once
#include <array>
#include "eventHandling.h"
#include "GUI/GUI.h"

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
