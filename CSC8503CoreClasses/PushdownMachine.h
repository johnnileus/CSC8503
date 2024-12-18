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
			void SetInMenu(bool m) { inMenu = m; }
			bool GetInMenu() { return inMenu; }
		protected:
			bool inMenu;
			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

