#pragma once
#include "PushdownMachine.h"


namespace NCL {
	namespace CSC8503 {
		class PushdownState {
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			};
			PushdownState()  {
			}
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc, bool isDead) = 0;
			virtual void OnAwake(PushdownMachine* inst) {}
			virtual void OnSleep(PushdownMachine* inst) {}



		};
	}
}