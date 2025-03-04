#include "TutorialGame.h"

#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"



using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");


	NetworkBase::Initialise();

	

	player = new Player("player");
	player->SetController(controller);
	player->SetCameraObject(&world->GetMainCamera());

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	catMesh		= renderer->LoadMesh("ORIGAMI_Chat.msh");
	kittenMesh	= renderer->LoadMesh("Kitten.msh");

	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete catMesh;
	delete kittenMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void CalculateCameraPosition(PerspectiveCamera* cam, Vector3 pos, float dist) {

	float yaw = cam->GetYaw();
	float rot = cam->GetPitch();

	Matrix3 yawMat = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));
	Matrix3 rotMat = Matrix::RotationMatrix3x3(rot, Vector3(1, 0, 0));

	Vector3 newDir = yawMat * rotMat * Vector3(0, 0, dist);

	cam->SetPosition(newDir + pos);

}

void TutorialGame::UpdateEnemy(float dt) {
	Vector3 plrPos = player->GetTransform().GetPosition();
	Vector3 enemyPos = maze.enemy->GetTransform().GetPosition();

	bool generatedMaze = true;
	if (!maze.pathFound) {
		generatedMaze =  maze.FindPath(enemyPos, plrPos);
	}
	if (generatedMaze) {
		if (maze.progress < maze.totalNodes - 1) {

			Vector3 directionToPlayer = plrPos - enemyPos;
			Ray ray = Ray(enemyPos, Vector::Normalise(directionToPlayer));
			Debug::DrawLine(enemyPos, enemyPos + Vector::Normalise(directionToPlayer), Debug::RED);
			RayCollision closestCollision;
			maze.chasingEnemy = false;
			if (world->Raycast(ray, closestCollision, true, maze.enemy)) {
				Debug::DrawLine(enemyPos, closestCollision.collidedAt, Vector4(1,0,0,1));
				Vector3 c = closestCollision.collidedAt;
				float dist = std::sqrt((plrPos.x - c.x) + (plrPos.y - c.y) + (plrPos.z - c.z));
				if (dist < 1.1f) {
					Debug::DrawLine(plrPos, enemyPos);
					maze.chasingEnemy = true;
				}
			}


			//turn into function
			if (maze.chasingEnemy) {
				maze.pathFound = false;
				generatedMaze = false;
				Vector3 dirToPlayer = Vector::Normalise(plrPos - enemyPos);
				float nodeSize = maze.grid.getNodeSize();
				maze.enemy->GetTransform().SetPosition(enemyPos + dirToPlayer * maze.speed * nodeSize * dt);
			}
			else {
				float nodeProgress = std::fmod(maze.progress, 1);
				int currentNode = maze.progress - nodeProgress;

				Vector3 nodePos = maze.nodes[currentNode];
				Vector3 nextNodePos = maze.nodes[currentNode + 1];

				Vector3 finalPos = Vector::Lerp(nodePos, nextNodePos, nodeProgress);
				maze.enemy->GetTransform().SetPosition(finalPos + Vector3(0, 1.8f, 0));

				maze.progress += maze.speed * dt;
			}


		}
		else {

			//turn into function
			if (maze.chasingEnemy) {
				maze.pathFound = false;
				generatedMaze = false;
				Vector3 dirToPlayer = Vector::Normalise(plrPos - enemyPos);
				float nodeSize = maze.grid.getNodeSize();
				maze.enemy->GetTransform().SetPosition(enemyPos + dirToPlayer * maze.speed * nodeSize * dt);
			}

			maze.pathFound = false;
			generatedMaze = false;
		}
	}




	maze.DisplayPath();

}

void TutorialGame::CheckIfPlayerGrounded() {
	Ray ray = Ray(player->GetTransform().GetPosition(), Vector3(0, -1, 0));
	RayCollision closestCollision;
	player->SetGrounded(false);
	if (world->Raycast(ray, closestCollision, true, player)) {
		if (closestCollision.rayDistance < 1.2f) {
			player->SetGrounded(true);
			Debug::DrawLine(player->GetTransform().GetPosition(), closestCollision.collidedAt, Vector4(.6f, 1, .6f, .5f), .1f);
		}
		
	}
}

void TutorialGame::UpdateConnection() {

	if (connected) {

		Vector3 posToSend = player->GetTransform().GetPosition();
		Quaternion rotToSend = player->GetTransform().GetOrientation();
		int scoreToSend = player->GetScore();
		PositionPacket p = PositionPacket(posToSend, rotToSend);
		HighscorePacket h = HighscorePacket(scoreToSend);

		if (isServer) {
			server->SendGlobalPacket(p);
			ghostPlayer.GO->GetTransform().SetPosition(serverReceiver.plrPos);
			ghostPlayer.GO->GetTransform().SetOrientation(serverReceiver.plrRot);

			server->SendGlobalPacket(h);
		}
		else {
			client->SendPacket(p);
			ghostPlayer.GO->GetTransform().SetPosition(clientReceiver.plrPos);
			ghostPlayer.GO->GetTransform().SetOrientation(clientReceiver.plrRot);
			client->SendPacket(h);

		}



		if (isServer) {
			server->UpdateServer();
		}
		else {
			client->UpdateClient();
		}
	}

	else {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::Y)) {
			server = new GameServer(port, 1);
			connected = true;
			isServer = true;

			server->RegisterPacketHandler(String_Message, &serverReceiver);
			server->RegisterPacketHandler(Message, &serverReceiver);
			server->RegisterPacketHandler(HighScore, &serverReceiver);
			CreateGhost(&ghostPlayer);
			std::cout << "created server\n";
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::U)) {
			client = new GameClient();
			connected = true;
			isServer = false;

			client->RegisterPacketHandler(String_Message, &clientReceiver);
			client->RegisterPacketHandler(Message, &clientReceiver);
			client->RegisterPacketHandler(HighScore, &clientReceiver);
			bool canConnect = client->Connect(127, 0, 0, 1, port);
			CreateGhost(&ghostPlayer);

			std::cout << "joined server\n";
		}
	}

	
	
}

void TutorialGame::UpdateGame(float dt) {

	maze.DisplayPath();

	if (player->kittensCollected == totalKittens) {
		menuMachine.collectedCats = true;
	}

	UpdateConnection();
	menuMachine.Update(dt);

	if (menuMachine.revivePlayer) {
		menuMachine.revivePlayer = false;
		player->kittensCollected = 0;
		totalKittens = 0;
		menuMachine.collectedCats = 0;
		InitWorld();
		player->SetDead(false);
		menuMachine.SetIsDead(false);

	}


	if (menuMachine.GetInMenu() == 0) { // in game
		if (!inSelectionMode) {
			world->GetMainCamera().UpdateCamera(dt);
		}


		//move camera rotation
		if (lockedObject != nullptr) {
			Vector3 objPos = lockedObject->GetTransform().GetPosition();
			Vector3 camPos = objPos + lockedOffset;

			Matrix4 temp = Matrix::View(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = Matrix::Inverse(temp);

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera().SetPosition(camPos);
			world->GetMainCamera().SetPitch(angles.x);
			world->GetMainCamera().SetYaw(angles.y);
		}

		UpdateKeys();

		if (useGravity) {
			Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
		}
		else {
			Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
		}

		Debug::Print("Score: " + std::to_string(player->GetScore()), Vector2(70, 5), Debug::YELLOW);
		if (connected) {
			Debug::Print("P2 Score: " + std::to_string(isServer ? serverReceiver.score: clientReceiver.score), Vector2(70,10), Debug::WHITE);

		}

		Debug::Print("Kittens: " + std::to_string(player->kittensCollected) + "/" + std::to_string(totalKittens), Vector2(70,15), Debug::YELLOW);

		//This year we can draw debug textures as well!
		//Debug::DrawTex(*basicTex, Vector2(10, 10), Vector2(5, 5), Debug::MAGENTA);

		RayCollision closestCollision;
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
			Vector3 rayPos;
			Vector3 rayDir;

			rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

			rayPos = selectionObject->GetTransform().GetPosition();

			Ray r = Ray(rayPos, rayDir);

			if (world->Raycast(r, closestCollision, true, selectionObject)) {
				if (objClosest) {
					objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				}
				objClosest = (GameObject*)closestCollision.node;

				objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
			}
		}

		Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

		SelectObject();
		MoveSelectedObject();

		UpdateEnemy(dt);


		player->UpdatePlayer(dt);
		CheckIfPlayerGrounded();

		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);

		menuMachine.SetIsDead(player->GetDead());

		//calculate position of camera for third person perspective
		CalculateCameraPosition(&world->GetMainCamera(), player->GetTransform().GetPosition(), 15.0f);

		renderer->Render();
		Debug::UpdateRenderables(dt);
	}
	else if (menuMachine.GetInMenu() == 1) { // main menu
		UpdateKeys();

		

		Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

		Debug::Print("Main Menu", Vector2(5, 6), Debug::BLUE);
		Debug::Print("Y to host server", Vector2(5, 12), Debug::CYAN);
		Debug::Print("U to join server", Vector2(5, 16), Debug::CYAN);
		Debug::Print("C to unpause", Vector2(5, 92), Debug::GREEN);

		SelectObject();
		MoveSelectedObject();

		//UpdateEnemy(dt);

		world->UpdateWorld(dt);
		renderer->Update(dt);
		//physics->Update(dt);

		//calculate position of camera for third person perspective
		CalculateCameraPosition(&world->GetMainCamera(), player->GetTransform().GetPosition(), 15.0f);

		renderer->Render();
		Debug::UpdateRenderables(dt);
	}
	else if (menuMachine.GetInMenu() == 2) {
		UpdateKeys();



		Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

		Debug::Print("Dead, C to restart", Vector2(5, 6), Debug::RED);


		SelectObject();
		MoveSelectedObject();

		//UpdateEnemy(dt);

		world->UpdateWorld(dt);
		renderer->Update(dt);
		//physics->Update(dt);

		//calculate position of camera for third person perspective
		CalculateCameraPosition(&world->GetMainCamera(), player->GetTransform().GetPosition(), 15.0f);

		renderer->Render();
		Debug::UpdateRenderables(dt);
	}
	else if (menuMachine.GetInMenu() == 3) {
		UpdateKeys();



		Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

		Debug::Print("Well done!, C to restart", Vector2(5, 6), Debug::CYAN);
		Debug::Print("Final Score: " + std::to_string(player->GetScore()), Vector2(5, 12), Debug::CYAN);


		SelectObject();
		MoveSelectedObject();

		//UpdateEnemy(dt);

		world->UpdateWorld(dt);
		renderer->Update(dt);
		//physics->Update(dt);

		//calculate position of camera for third person perspective
		CalculateCameraPosition(&world->GetMainCamera(), player->GetTransform().GetPosition(), 15.0f);

		renderer->Render();
		Debug::UpdateRenderables(dt);
	}


	
}

void TutorialGame::UpdateKeys() {



	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the 
// 
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}



	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld	= Matrix::Inverse(view);

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);


	InitDefaultFloor();
	BridgeConstraintTest();

	player = new Player("player");
	player->SetController(controller);
	player->SetCameraObject(&world->GetMainCamera());

	GenerateMaze();

	CreateObjectToPlayer(player);
	CreateObjectToEnemy();

}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2.0f)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

void TutorialGame::GenerateMaze() {
	int w = maze.grid.GetWidth();
	int h = maze.grid.GetHeight();
	float size = maze.grid.getNodeSize();
	GridNode* allNodes = maze.grid.GetAllNodes();

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			int ind = (w * y) + x;
			GridNode node = allNodes[(w * y) + x];
			char type = allNodes[ind].type;
			if (type == 'x') {
				AddCubeToWorld(node.position, Vector3(size /2, 5, size/2), 0.0f);
			}
			else {
				int num = std::rand() % 100;
				if (num < 2) {
					AddBonusToWorld(node.position + Vector3(0, 1, 0));
				}
				else if (num < 3){
					AddKittenToWorld(node.position + Vector3(0, 1, 0));
					totalKittens++;
				}
			}
		}
	}
}

GameObject* TutorialGame::CreateObjectToPlayer(Player* plr) {
	Vector3 size = Vector3(1.0f, 1.0f, 1.0f);
	SphereVolume* volume = new SphereVolume(1.0f);
	plr->SetBoundingVolume((CollisionVolume*)volume);

	plr->GetTransform()
		.SetPosition({30.0f,-15.0f,-5.0f})
		.SetScale(size * 2.0f);

	plr->SetRenderObject(new RenderObject(&plr->GetTransform(), catMesh, basicTex, basicShader));
	plr->SetPhysicsObject(new PhysicsObject(&plr->GetTransform(), plr->GetBoundingVolume()));
	plr->GetPhysicsObject()->SetElasticity(0.0f);

	plr->GetPhysicsObject()->SetInverseMass(1.0f);
	plr->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(plr);
	return plr;
}

GameObject* TutorialGame::CreateObjectToEnemy() {
	GameObject* model = new GameObject("enemy");
	Vector3 size = Vector3(1.0f, 1.0f, 1.0f);
	SphereVolume* volume = new SphereVolume(1.0f);
	model->SetBoundingVolume((CollisionVolume*)volume);

	model->GetTransform()
		.SetPosition({ 40.0f,-15.0f,40.0f })
		.SetScale(size * 2.0f);
	RenderObject* enemy = new RenderObject(&model->GetTransform(), enemyMesh, basicTex, basicShader);
	enemy->SetColour({ 1,0,0,1 });
	model->SetRenderObject(enemy);
	model->SetPhysicsObject(new PhysicsObject(&model->GetTransform(), model->GetBoundingVolume()));
	model->GetPhysicsObject()->SetElasticity(0.0f);

	model->GetPhysicsObject()->SetInverseMass(1.0f);
	model->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(model);
	maze.enemy = model;
	return model;
}

GameObject* TutorialGame::CreateGhost(GhostPlayer* plr) {
	GameObject* model = new GameObject();
	Vector3 size = Vector3(1.0f, 1.0f, 1.0f);
	SphereVolume* volume = new SphereVolume(1.0f);
	model->SetBoundingVolume((CollisionVolume*)volume);

	model->GetTransform()
		.SetPosition({ 0.0f,-5.0f,0.0f })
		.SetScale(size * 2.0f);
	RenderObject* cat = new RenderObject(&model->GetTransform(), catMesh, basicTex, basicShader);
	cat->SetColour({ 0,0,1,1 });
	model->SetRenderObject(cat);
	model->SetPhysicsObject(new PhysicsObject(&model->GetTransform(), model->GetBoundingVolume()));
	model->GetPhysicsObject()->SetElasticity(0.0f);

	model->GetPhysicsObject()->SetInverseMass(1.0f);
	model->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(model);
	plr->GO = model;
	return model;
}


GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

//not used
GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();
	SphereVolume* volume  = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), catMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject("enemy");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("bonus");

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);
	RenderObject* renderObject = new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader);
	renderObject->SetColour(Vector4(1, 1, 0, 1));
	apple->SetRenderObject(renderObject);
	PhysicsObject* physicsObject = new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume());
	physicsObject->SetGravity(false);
	apple->SetPhysicsObject(physicsObject);

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

GameObject* TutorialGame::AddKittenToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("kitten");

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);
	RenderObject* renderObject = new RenderObject(&apple->GetTransform(), kittenMesh, nullptr, basicShader);
	renderObject->SetColour(Vector4(1, 0, 1, 1));
	apple->SetRenderObject(renderObject);
	PhysicsObject* physicsObject = new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume());
	physicsObject->SetGravity(false);
	apple->SetPhysicsObject(physicsObject);

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {


		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	
	if (!menuMachine.GetInMenu()) {
		Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	}
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 10.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(
					ray.GetDirection() * forceMagnitude,
					closestCollision.collidedAt
				);
			}
		}

	}

}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(2, 2, 8);

	float invCubeMass = 5; // how heavy the middle pieces are
	int numLinks = 5;
	float maxDistance = 8; // constraint distance
	float cubeDistance = 4; // distance between links

	Vector3 startPos = Vector3(-15,-12,-15);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0),
		cubeSize, 0);
	Vector3 endPos = Vector3((numLinks + 2) * cubeDistance,
		(numLinks + 2) * cubeDistance / 2, 0);
	GameObject* end = AddCubeToWorld(startPos + endPos, cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) *
			cubeDistance, (i + 1) *
			cubeDistance / 2, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous,
			block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous,
		end, maxDistance);
	world->AddConstraint(constraint);

	AddKittenToWorld(endPos + startPos + Vector3(0, 2, 0));
	totalKittens++;

}
