#pragma once
#include "GameServer.h"
#include "GameClient.h"


#include <iostream>

struct PositionPacket : public GamePacket {
	Vector3 pos;

	PositionPacket(const Vector3 p) {
		type = BasicNetworkMessages::Message;
		size = sizeof(pos);
		pos = p;
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
			std::cout << "pos:" << realPacket->pos.x << realPacket->pos.y << realPacket->pos.z << std::endl;
		}
	}

protected:
	std::string name;
};

class GhostPlayer {
public:
	Vector3 position;
};




//struct StringPacket : public GamePacket {
//	char stringData[256];
//
//	StringPacket(const std::string& message) {
//		type = BasicNetworkMessages::String_Message;
//		size = (short)message.length();
//		memcpy(stringData, message.data(), size);
//	}
//
//	std::string GetStringFromData() {
//		std::string realString(stringData);
//		realString.resize(size);
//		return realString;
//	}
//};