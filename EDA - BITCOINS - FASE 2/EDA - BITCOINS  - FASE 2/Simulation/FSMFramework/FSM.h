#pragma once
#include "genericFSM.h"
#include "eventHandling.h"

/*Function cast.*/
#define TX(x) (static_cast<void (genericFSM::* )(genericEvent *)>(&FSM::x))

namespace eventStates {
	const enum eventStates : stateTypes {
		NOTHING,
		INITIALIZED,
		END
		/*...7500 more states*/
	};
}
using namespace std;
class FSM : public genericFSM
{
public:
	FSM() : genericFSM(&fsmTable[0][0], 4, 4, eventStates::NOTHING) {}

private:

	const fsmCell fsmTable[4][4] = {
		//       EventA                 EventB                  EventC                  EventD
		{  	{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)}},   //State0
		{	{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)}},   //State1
		{	{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)}},   //State2
		{	{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)},		{eventStates::END,TX(test)}}    //State3
	};

	void test(genericEvent* ev) {
		return;
	}
};
