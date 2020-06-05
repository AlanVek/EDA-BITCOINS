#pragma once
#include "GUI.h"
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

	void perform(void);

	const unsigned int getIndex(void);
	/*Prevents from using copy constructor.*/
	Simulation(const Simulation&);

	GUI* gui;

	std::vector <Node*> nodes;

	bool running;

	boost::asio::io_context io_context;

	Events ev;
};
