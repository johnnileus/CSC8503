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


};

struct HighscorePacket : public GamePacket {
	int score;

	HighscorePacket(const int s) {
		type = BasicNetworkMessages::HighScore;
		size = sizeof(int);
		score = s;
	}

};

class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(std::string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		//std::cout << "message type: " << type << std::endl;
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;
			std::string msg = realPacket->GetStringFromData();
		}

		if (type == Message) {
			PositionPacket* realPacket = (PositionPacket*)payload;
			plrPos = realPacket->pos;
			plrRot = realPacket->rot;
		}
		if (type == HighScore) {
			std::cout << "BWAH\n";
			HighscorePacket* realPacket = (HighscorePacket*)payload;
			score = realPacket->score;
		}
	}

	Quaternion plrRot;
	Vector3 plrPos;
	int score;

protected:
	std::string name;
};

struct GhostPlayer {
	GameObject* GO;
};