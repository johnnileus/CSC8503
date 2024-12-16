#include "Player.h"



using namespace NCL;
using namespace CSC8503;



Player::Player() {

}

void Player::UpdatePlayer(float dt) {

	float yaw = camera->GetYaw();

	Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));

	float frameSpeed = dt * 5.0f;

	Vector3 impulse = Vector3();
	
	impulse += yawRotation * Vector3(0, 0, -activeController->GetNamedAxis("Forward")) * frameSpeed;
	impulse += yawRotation * Vector3(activeController->GetNamedAxis("Sidestep"), 0, 0) * frameSpeed;

	impulse.y += activeController->GetNamedAxis("UpDown") * frameSpeed;

	gameObject->GetPhysicsObject()->ApplyLinearImpulse(impulse);
}

