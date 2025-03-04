#include "PushdownMachine.h"
#include "PushdownState.h"

using namespace NCL::CSC8503;

PushdownMachine::PushdownMachine(PushdownState* initialState)
{
	this->initialState = initialState;
}

PushdownMachine::~PushdownMachine()
{
}

bool PushdownMachine::Update(float dt) {
	if (activeState) {
		PushdownState* newState = nullptr;
		PushdownState::PushdownResult result = activeState->OnUpdate(dt, &newState, isDead, collectedCats);

		switch (result) {
			case PushdownState::Pop: {
				activeState->OnSleep(this);
				delete activeState;
				stateStack.pop();
				if (stateStack.empty()) {
					return false;
				}
				else {
					activeState = stateStack.top();
					activeState->OnAwake(this);
				}					
			}break;
			case PushdownState::Push: {
				activeState->OnSleep(this);		

				stateStack.push(newState);
				activeState = newState;
				activeState->OnAwake(this);
			}break;
		}
	}
	else {
		stateStack.push(initialState);
		activeState = initialState;
		activeState->OnAwake(this);
	}



	return true;
}