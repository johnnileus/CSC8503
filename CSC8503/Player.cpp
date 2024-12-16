#include "Player.h"


using namespace NCL;
using namespace CSC8503;

#define PI 3.141592

Player::Player() {

}

void Player::UpdatePlayer(float dt) {





	bool inputting = false;
	float forwardInput = activeController->GetNamedAxis("Forward");
	float sideInput = activeController->GetNamedAxis("Sidestep");
	float verticalInput = activeController->GetNamedAxis("UpDown");

	if (!forwardInput && !sideInput) {
		inputting = false;
	}
	else {
		inputting = true;
	}


	//wasd position
	float frameSpeed = dt * 5.0f;
	float yaw = camera->GetYaw();
	Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));
	Vector3 impulse = Vector3();
	impulse += yawRotation * Vector3(0, 0, -forwardInput) * frameSpeed;
	impulse += yawRotation * Vector3(sideInput, 0, 0) * frameSpeed;
	impulse.y += verticalInput * frameSpeed;
	gameObject->GetPhysicsObject()->ApplyLinearImpulse(impulse);
	//cap speed, make friction

	//lerp model to forward
	if (inputting) {

		//convert input to yaw
		Vector2 input = Vector2(sideInput, forwardInput);
		input = Vector::Normalise(input);

		float yawOffset = atan2(input.x, input.y) * 180.0f / PI;


		Quaternion target = Quaternion::EulerAnglesToQuaternion(0, yaw - yawOffset + 180, 0);
		Quaternion current = gameObject->GetTransform().GetOrientation();

		Quaternion newRot = Quaternion::Lerp(current, target, 10.0f * dt);

		gameObject->GetTransform().SetOrientation(newRot);

	}
	

}

