#pragma once
#include "FSMFramework/GUI/GUI.h"
#include "Nodes/Node/Node.h"

class Simulation {
public:
	Simulation(void);
	~Simulation(void);

	void dispatch(const Events&);

	void mainScreen(void);

	const Events eventGenerator();

	bool isRunning(void);

private:

	/*Prevents from using copy constructor.*/
	Simulation(const Simulation&);

	GUI* gui;

	std::vector <Node*> nodes;

	bool running;

	boost::asio::io_context io_context;
};
