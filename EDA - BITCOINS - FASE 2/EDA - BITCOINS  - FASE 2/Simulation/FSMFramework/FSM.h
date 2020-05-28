#pragma once
#include "FSMFramework/genericFSM.h"
#include "FSMFramework/eventHandling.h"
#include <iostream>

enum implStates : stateTypes { State0, State1, State2, State3 };
using namespace std;
class FSM : public genericFSM
{
public:
	FSM() : genericFSM(&fsmTable[0][0], 4, 4, State0) {}

private:

#define TX(x) (static_cast<void (genericFSM::* )(genericEvent *)>(&FSM::x)) //casteo a funcion, por visual
	const fsmCell fsmTable[4][4] = {
		//       EventA                 EventB                  EventC                  EventD
		{  	{State0,TX(prueba1)},		{State1,TX(prueba2)},		{State2,TX(prueba3)},		{State3,TX(prueba4)}},   //State0
		{	{State1,TX(prueba1)},		{State2,TX(prueba2)},		{State3,TX(prueba3)},		{State0,TX(prueba4)}},   //State1
		{	{State2,TX(prueba1)},		{State3,TX(prueba2)},		{State0,TX(prueba3)},		{State1,TX(prueba4)}},   //State2
		{	{State3,TX(prueba1)},		{State0,TX(prueba2)},		{State1,TX(prueba3)},		{State2,TX(prueba4)}}    //State3
	};

	//The action routines for the FSM
	//These actions should not generate fsmEvents

	void prueba1(genericEvent* ev)
	{
		cout << "prueba 1" << endl;
		return;
	}
	void prueba2(genericEvent* ev)
	{
		cout << "prueba 2" << endl;
		return;
	}
	void prueba3(genericEvent* ev)
	{
		cout << "prueba 3" << endl;
		return;
	}
	void prueba4(genericEvent* ev)
	{
		cout << "prueba 4" << endl;
		return;
	}
};
