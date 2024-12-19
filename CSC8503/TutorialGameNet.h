#pragma once
#include "GameServer.h"
#include "GameClient.h"


#include <iostream>

struct PositionPacket : public GamePacket {
	Vector3 pos;
	Quaternion rot;

	PositionPacket(const Vector3 p, const Quaternion r) {
		type = BasicNetworkMessages::Message;
		size = sizeof(Vector3) + sizeof(Quaternion);
		pos = p;
		rot = r;
	}
	Vector3 getPosFromData() {
		return pos;
	}

};

class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(std::string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		std::cout << "message type: " << type << std::endl;
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;
			std::string msg = realPacket->GetStringFromData();
			std::cout << name << " received message: " << msg << std::endl;
		}

		if (type == Message) {
			PositionPacket* realPacket = (PositionPacket*)payload;
			plrPos = realPacket->pos;
			plrRot = realPacket->rot;
		}
	}

	Quaternion plrRot;
	Vector3 plrPos;

protected:
	std::string name;
};

class GhostPlayer {
public:
	GameObject* GO;

};