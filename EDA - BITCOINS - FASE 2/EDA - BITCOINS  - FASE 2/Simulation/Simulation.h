#pragma once
#include "FSMFramework/GUI/GUI.h"

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

	bool running;
};
