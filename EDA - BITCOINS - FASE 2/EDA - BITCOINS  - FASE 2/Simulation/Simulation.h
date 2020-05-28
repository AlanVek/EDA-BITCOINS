#pragma once
#include "Nodes/SVP_Node.h"
#include "Nodes/Full_Node.h"
#include "Nodes/Node/Node.h"
#include "FSMFramework/GUI/GUI.h"
#include "FSMFramework/FSM.h"

class Simulation {
public:
	Simulation(void);
	~Simulation(void);

	void dispatch(const Events&);

	const Events eventGenerator();

	bool isRunning(void);

private:

	/*Prevents from using copy constructor.*/
	Simulation(const Simulation&);

	void newInfoDispatch(const BlockInfo&);

	std::vector <Node*> nodes;

	GUI* gui;

	bool running;
};
