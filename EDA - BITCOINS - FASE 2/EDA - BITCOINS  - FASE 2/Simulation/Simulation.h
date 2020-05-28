#pragma once
#include "BlockChain/BlockChain.h"
#include "GUI/GUI.h"

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

	BlockChain blockChain;

	GUI* gui;

	bool running;
};
