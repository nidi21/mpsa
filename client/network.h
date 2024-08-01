#pragma once
#include "main.h"
#include "dependencies/netcode/netcode.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <thread>
#include <functional>
#include <iostream>
#include <CStreaming.h>
#include "CCivilianPed.h"
#include "CPopulation.h"
#include "CAnimManager.h"
#include "CWorld.h"
#include "eScriptCommands.h"
#include <iostream>
#include "extensions/ScriptCommands.h"
#include <chrono>
#include <map>

#include "CTaskComplexWanderStandard.h"

enum PacketID {
	SPAWN_PLAYER = 0,
	DELETE_PLAYER = 1,
	FOOT_SYNC = 2,
	CHAT_MESSAGE = 3,
};

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

// PACKETS
struct pCreateLocalPlayer // ONLY SEND (CLIENT)
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
	CPed* ped;
	std::chrono::time_point<std::chrono::system_clock> lastOnFootSync;
};

namespace network {
	extern std::thread thread;
	extern std::map<int, Player> players;
	extern bool id_in_map(int id);
	extern void init();
	extern void shutdown();
	extern bool inited;


	extern bool connect(std::string address);
	extern void disconnect();
	extern bool is_connected();
	extern void update();

	extern void spawn_local_player(std::string name);
	extern void send_foot_sync();

	void handle_packet(void* packet);
	void receive_packets();	
	void send_packet(int id, void* packet, int packet_bytes);
}