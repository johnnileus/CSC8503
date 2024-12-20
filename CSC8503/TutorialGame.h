#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "Player.h"

#include "MenuAutomata.h"
#include "TutorialGameNet.h"
#include "Maze.h"



namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:

			static TutorialGame& GetInstance() {
				static TutorialGame instance;
				return instance;
			}

			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void CheckIfPlayerGrounded();

			void UpdateConnection();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* CreateObjectToPlayer(Player* plr);
			GameObject* CreateObjectToEnemy();
			GameObject* CreateGhost(GhostPlayer* plr);

			void UpdateEnemy(float dt);
			void GenerateMaze();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddKittenToWorld(const Vector3& position);


			void BridgeConstraintTest();


#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;
			Maze maze;
			int totalKittens = 0;

			////networking
			bool connected = false;
			bool isServer = false;
		
			TestPacketReceiver serverReceiver{ "Server" };
			TestPacketReceiver clientReceiver{ "Client" };

			GameServer* server = nullptr;
			GameClient* client = nullptr;

			int port = NetworkBase::GetDefaultPort();

			GhostPlayer ghostPlayer;

			MenuMachine menuMachine = MenuMachine(new MainMenuState());
			KeyboardMouseController controller;

			Player* player;


			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;

			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	catMesh		= nullptr;
			Mesh*	kittenMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			

			GameObject* objClosest = nullptr;
		};
	}
}


