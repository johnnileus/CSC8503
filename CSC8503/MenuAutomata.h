#pragma once
#include "PushdownMachine.h"
#include "Window.h"
#include "PushdownState.h"

using namespace NCL;
using namespace CSC8503;





class InGameState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			return PushdownResult::Pop;
		}

		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		std::cout << "Unpaused\n";
	}
protected:

};

class MainMenuState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			*newState = new InGameState();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override {
		std::cout << "Game Paused\n";
	}
	
};


//END OF STATES

class MenuMachine : public PushdownMachine {
public:
	MenuMachine(PushdownState* initialState) : PushdownMachine(initialState), inMenu(false) {
		this->initialState = initialState;
	}

	void SetInMenu(bool m) { inMenu = m; }
	bool GetInMenu() { return inMenu; }
protected:
	bool inMenu;
};