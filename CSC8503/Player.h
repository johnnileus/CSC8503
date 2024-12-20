#pragma once
#include "GameObject.h"
#include "Controller.h"
#include "Matrix.h"
#include "Vector.h"
#include "PhysicsObject.h"
#include "Camera.h"
#include "Ray.h"
#include <math.h>



namespace NCL {
	namespace CSC8503 {
		class Player : public GameObject {
		public:
			Player(const std::string& objectName = "") { name = objectName; }

			void UpdatePlayer(float dt);

			void SetCameraObject(PerspectiveCamera* cam) { camera = cam; }


			void SetController(const Controller& c) {
				activeController = &c;
			}

			void SetGrounded(bool g) { grounded = g; }

			void IncrementScore(int s) { score += s; std::cout << "bwu\n"; }
			void SetScore(int s) { score = s; }
			int GetScore() { return score; }

			virtual void OnCollisionBegin(GameObject* otherObject);
			
			bool GetDead() { return dead; }
			void SetDead(bool d) { dead = d; }
			int kittensCollected = 0;

		protected:
			const Controller* activeController = nullptr;
			PerspectiveCamera* camera;
			bool grounded;
			int score = 0;

			bool dead = false;

		};
	}
}