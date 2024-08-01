#pragma once

#include <iostream>

#include "dependencies/netcode/netcode.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <map>
#include <inttypes.h>

#define TEST_PROTOCOL_ID 0x1122334455667788

enum PacketID {
	SPAWN_PLAYER = 0,
	DELETE_PLAYER = 1,
	FOOT_SYNC = 2,
	CHAT_MESSAGE = 3,
};

//PACKETS

struct Position
{
	float x, y, z;
};

struct ChatMessage {
	short id;
	char name[20];
	char message[200];
};

struct OnFootSync {
	short id;
	Position pos;
	float heading;
	int modelIndex;
	char name[20];
	unsigned int anim_group;
	unsigned int animid;
};

struct pCreateLocalPlayer // ONLY RECEIVE (SERVER)
{
	char name[20];
	int modelIndex;
	Position pos;
};

// STRUCTS

struct Player
{
	short id;
	char name[20];
	Position pos;
	int modelIndex;
};

namespace network {
	extern void init();

	extern void update();

	void handle_packet(void* packet, int client_author);
	void send_packet(int client, int id, void* packet, int packet_bytes);
	void send_packet_except(int client, int id, void* packet, int packet_bytes);
	void send_packet_nearby(int client, int id, void* packet, int packet_bytes);
	void send_packet_everyone(int id, void* packet, int packet_bytes);
	extern void shutdown();
}