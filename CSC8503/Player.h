#pragma once
#include "GameObject.h"
#include "Controller.h"
#include "Matrix.h"
#include "Vector.h"
#include "PhysicsObject.h"
#include "Camera.h"
#include <math.h>



namespace NCL {
	namespace CSC8503 {
		class Player {
		public:
			Player();

			void UpdatePlayer(float dt);
			void SetGameObject(GameObject* GO) { gameObject = GO; }

			void SetCameraObject(PerspectiveCamera* cam) { camera = cam; }

			GameObject* GetGameObject() { return gameObject;}

			void SetController(const Controller& c) {
				activeController = &c;
			}

		protected:
			GameObject* gameObject;
			const Controller* activeController = nullptr;
			PerspectiveCamera* camera;

		};
	}
}