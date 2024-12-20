#pragma once

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine
		{
		public:
			PushdownMachine(PushdownState* initialState);
			~PushdownMachine();

			bool Update(float dt);
			void SetInMenu(int m) { inMenu = m; }
			int GetInMenu() { return inMenu; }

			void SetIsDead(bool d) { isDead = d; }
			bool GetIsDead() { return isDead; }
			bool revivePlayer = false;
			bool collectedCats = false;
		protected:
			bool isDead;
			int inMenu;
			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

