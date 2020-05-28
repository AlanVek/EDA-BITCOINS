#pragma once
#include <array>
#include "eventHandling.h"

enum implEvent : eventTypes { EventA, EventB, EventC, EventD, EventQuit };

class cEventA : public genericEvent
{
public:
	eventTypes getType(void) { return EventA; }
};

class cEventB : public genericEvent
{
public:
	eventTypes getType(void) { return EventB; }
};

class cEventC : public genericEvent
{
public:
	eventTypes getType(void) { return EventC; }
};

class cEventD : public genericEvent
{
public:
	eventTypes getType(void) { return EventD; }
};

class cEventQuit : public genericEvent
{
public:
	eventTypes getType(void) { return EventQuit; }
};

class GUIEvents : public eventGenerator
{
public:
	GUIEvents();
	genericEvent* getEvent(void);

private:

	unsigned int pos;
	const std::array<implEvent, 13> testEvs = { EventB,EventB,EventB,EventD,EventD,EventD,EventC,EventA,EventB,EventA,EventB,EventD,EventQuit };
};
