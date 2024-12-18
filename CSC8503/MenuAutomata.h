#pragma once
#include "PushdownMachine.h"
#include "Window.h"
#include "PushdownState.h"

using namespace NCL;
using namespace CSC8503;





class InGameState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
			return PushdownResult::Pop;
		}

		return PushdownResult::NoChange;
	}

	void OnAwake(PushdownMachine* inst) override {
		Window::GetWindow()->ShowOSPointer(false);
		Window::GetWindow()->LockMouseToWindow(true);
		inst->SetInMenu(false);
	}
protected:

};

class MainMenuState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
			*newState = new InGameState();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(PushdownMachine* inst) override {
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		inst->SetInMenu(true);
	}
	
};


//END OF STATES

class MenuMachine : public PushdownMachine {
public:
	MenuMachine(PushdownState* initialState) : PushdownMachine(initialState) {
		this->initialState = initialState;
	}


};