#pragma once
#include "PushdownMachine.h"
#include "Window.h"
#include "PushdownState.h"

using namespace NCL;
using namespace CSC8503;




class DeadState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState, bool isDead, bool collectedCats) override {

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(PushdownMachine* inst) override {
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		inst->SetInMenu(2);
	}

	void OnSleep(PushdownMachine* inst) override {
		inst->revivePlayer = true;
	}

};

class WinState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState, bool isDead, bool collectedCats) override {

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(PushdownMachine* inst) override {
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		inst->SetInMenu(3);
	}

	void OnSleep(PushdownMachine* inst) override {
		inst->revivePlayer = true;
	}

};

class InGameState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState, bool isDead, bool collectedCats) override {
		std::cout << isDead << std::endl;
		if (isDead) {
			*newState = new DeadState();
			return PushdownResult::Push;
		}
		if (collectedCats) {
			*newState = new WinState();
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
			return PushdownResult::Pop;
		}

		return PushdownResult::NoChange;
	}

	void OnAwake(PushdownMachine* inst) override {
		
		Window::GetWindow()->ShowOSPointer(false);
		Window::GetWindow()->LockMouseToWindow(true);
		inst->SetInMenu(0);
	}
protected:

};


class MainMenuState : public PushdownState {
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState, bool isDead, bool collectedCats) override {

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
			*newState = new InGameState();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(PushdownMachine* inst) override {
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
		inst->SetInMenu(1);
	}
	
};


//END OF STATES

class MenuMachine : public PushdownMachine {
public:
	MenuMachine(PushdownState* initialState) : PushdownMachine(initialState) {
		this->initialState = initialState;
	}
	

};