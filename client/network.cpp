#include "network.h"
#include "utils.h"
#include "chat.h"
#include <format>

bool network::inited = false;
std::thread network::thread;

#define CONNECT_TOKEN_EXPIRY 30
#define CONNECT_TOKEN_TIMEOUT 5
#define PROTOCOL_ID 0x1122334455667788

static uint8_t private_key[NETCODE_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea,
                                                  0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4,
                                                  0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                  0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };
double time_ = 0.0;
double delta_time = 1.0 / 60.0;

struct netcode_client_t* client = nullptr;

static uint8_t packet_data[NETCODE_MAX_PACKET_SIZE];

Player players[NETCODE_MAX_CLIENTS] = { };
std::map<int, Player> network::players;

bool network::id_in_map(int id) {
    return !(players.find(id) == players.end());
}

void interrupt_handler(int signal)
{
    (void)signal;
}

void network::init() {
    if (netcode_init() != NETCODE_OK)
    {
        printf("error: failed to initialize netcode\n");
        return;
    }
    
    netcode_log_level(NETCODE_LOG_LEVEL_INFO);

    struct netcode_client_config_t client_config;
    netcode_default_client_config(&client_config);
    client = netcode_client_create("0.0.0.0", &client_config, time_);

    if (!client)
    {
        printf("error: failed to create client\n");
        return;
    }

    printf("success: netcode initialized\n");

    signal(SIGINT, interrupt_handler);

    thread = std::thread(network::update);

    inited = true;
}

bool network::is_connected() {
    if (!client)
        return false;
    return netcode_client_state(client) == NETCODE_CLIENT_STATE_CONNECTED;
}

bool network::connect(std::string address) {
    if (!client) {
        printf("client not initialized, cannot connect\n");
        return false;
    }

    const char* server_address = address.c_str();

    uint64_t client_id = 0;
    netcode_random_bytes((uint8_t*)&client_id, 8);
    printf("client id is %.16" PRIx64 "\n", client_id);

    uint8_t user_data[NETCODE_USER_DATA_BYTES];
    netcode_random_bytes(user_data, NETCODE_USER_DATA_BYTES);

    uint8_t connect_token[NETCODE_CONNECT_TOKEN_BYTES];

    if (netcode_generate_connect_token(1, &server_address, &server_address, CONNECT_TOKEN_EXPIRY, CONNECT_TOKEN_TIMEOUT, client_id, PROTOCOL_ID, private_key, user_data, connect_token) != NETCODE_OK)
    {
        printf("error: failed to generate connect token\n");
        return false;
    }

    netcode_client_connect(client, connect_token);
    return true;
}

void network::disconnect() {
    if (!client && !is_connected())
        return;

    for (int i = 0; i < 256; i++) {
        if (id_in_map(i))
        {
            CWorld::Remove(players[i].ped);
            players.erase(i);
        }
    }

    //CPopulation::RemoveAllRandomPeds();
    chat::messages.clear();
    netcode_client_disconnect(client);
}

void network::spawn_local_player(std::string name) {
    pCreateLocalPlayer player;
    
    auto pos = FindPlayerPed()->GetPosition();
    
    player.modelIndex = 106;
    player.pos = Position({pos.x, pos.y, pos.z});
    strcpy_s(player.name, name.c_str());

    send_packet(SPAWN_PLAYER, &player, sizeof(pCreateLocalPlayer));
}

void network::send_foot_sync() {
    OnFootSync sync;
    auto pos = FindPlayerPed()->GetPosition();

    const auto me = FindPlayerPed();
    CAnimBlendAssociation* cacat = nullptr;
    float pl = 0;
    const auto assoc = RpAnimBlendClumpGetMainAssociation(me->m_pRwClump, &cacat, &pl);

    sync.id = 0;
    sync.heading = FindPlayerPed()->GetHeading();
    sync.pos = Position({ pos.x, pos.y, pos.z });
    sync.animid = assoc->m_nAnimId;
    sync.anim_group = assoc->m_nAnimGroup;
        
    send_packet(FOOT_SYNC, &sync, sizeof(OnFootSync));
}

void network::handle_packet(void* packet) {
    const auto packet_data = (uint8_t*)packet;
    const PacketID id = (PacketID)packet_data[0];
    switch (id)
    {
    case SPAWN_PLAYER:
    {
        Player player = *reinterpret_cast<Player*>(packet_data + sizeof(uint8_t));

        if (id_in_map(player.id))
            return;

        CStreaming::RequestModel(player.modelIndex, 2);
        CStreaming::LoadAllRequestedModels(false);
        player.ped = new CCivilianPed(CPopulation::IsFemale(player.modelIndex) ? PED_TYPE_CIVFEMALE : PED_TYPE_CIVMALE, player.modelIndex);
        if (player.ped) {

            player.ped->SetModelIndex(player.modelIndex);
            player.ped->SetPosn(player.pos.x, player.pos.y, player.pos.z);
            CWorld::Add(player.ped);
        }

        memcpy(&players[player.id], &player, sizeof(Player));
        break;
    }
    case DELETE_PLAYER:
    {
        auto player_id = *reinterpret_cast<int*>(packet_data + sizeof(uint8_t));

        if (!id_in_map(player_id))
            return;

        if (players[player_id].ped)
            CPopulation::RemovePed(players[player_id].ped);

        players.erase(player_id);
        break;
    }
    case FOOT_SYNC:
    {
        OnFootSync data = *reinterpret_cast<OnFootSync*>(packet_data + sizeof(uint8_t));
        if (id_in_map(data.id) && players[data.id].ped)
        {
            auto dist = get_distance({ data.pos.x, data.pos.y, data.pos.z });
            if (dist >= 150)
            { 
                players[data.id].ped->SetPosn(0.f,0.f,0.f);
                return;
            }

            players[data.id].ped->SetPosn(data.pos.x, data.pos.y, data.pos.z);
            players[data.id].ped->m_fAimingRotation = players[data.id].ped->m_fCurrentRotation = data.heading;
            players[data.id].ped->SetHeading(data.heading);
            players[data.id].ped->UpdateRwMatrix();

            CAnimManager::AddAnimation(players[data.id].ped->m_pRwClump, data.anim_group, data.animid);
        }
        else if (!id_in_map(data.id)) {
            Player player;
            player.id = data.id;
            player.modelIndex = data.modelIndex;
            player.pos = data.pos;
            player.ped = nullptr;
            strcpy_s(player.name, data.name);

            auto dist = get_distance({ data.pos.x, data.pos.y, data.pos.z });
            if (dist <= 150) {
                CStreaming::RequestModel(player.modelIndex, 2);
                CStreaming::LoadAllRequestedModels(false);

                player.ped = new CCivilianPed(CPopulation::IsFemale(data.modelIndex) ? PED_TYPE_CIVFEMALE : PED_TYPE_CIVMALE, data.modelIndex);

                if (player.ped) {

                    player.ped->SetModelIndex(data.modelIndex);
                    player.ped->SetPosn(data.pos.x, data.pos.y, data.pos.z);
                    player.ped->m_fAimingRotation = player.ped->m_fCurrentRotation = data.heading;
                    player.ped->SetHeading(data.heading);
                    player.ped->UpdateRwMatrix();                    
                    CWorld::Add(player.ped);
                }
            }

            memcpy(&players[data.id], &player, sizeof(Player));
        }
        else if (id_in_map(data.id) && !players[data.id].ped) {
            auto dist = get_distance({ data.pos.x, data.pos.y, data.pos.z });
            if (dist >= 150)
                return;

            CStreaming::RequestModel(data.modelIndex, 2);
            CStreaming::LoadAllRequestedModels(false);
            players[data.id].ped = new CCivilianPed(CPopulation::IsFemale(data.modelIndex) ? PED_TYPE_CIVFEMALE : PED_TYPE_CIVMALE, data.modelIndex);
            if (players[data.id].ped) {
                players[data.id].ped->SetModelIndex(data.modelIndex);

                players[data.id].ped->SetPosn(data.pos.x, data.pos.y, data.pos.z);
                players[data.id].ped->m_fAimingRotation = players[data.id].ped->m_fCurrentRotation = data.heading;
                players[data.id].ped->SetHeading(data.heading);
                players[data.id].ped->UpdateRwMatrix();
                CWorld::Add(players[data.id].ped);
            }
        }

        if (id_in_map(data.id)) {
            players[data.id].lastOnFootSync = std::chrono::system_clock::now();
        }
        break;
    }
    case CHAT_MESSAGE:
    {
        ChatMessage data = *reinterpret_cast<ChatMessage*>(packet_data + sizeof(uint8_t));
        auto message = std::format("{}[{}]: {}", data.name, data.id, data.message);
        chat::messages.emplace_back(message);
        break;
    }
    }
}

void network::receive_packets()
{
    int packet_bytes;
    uint64_t packet_sequence;
    
    void* packet = netcode_client_receive_packet(client, &packet_bytes, &packet_sequence);
    
    if (!packet)
        return;

    handle_packet(packet);

    netcode_client_free_packet(client, packet);
}

void network::send_packet(int id, void* packet, int packet_bytes) {
    uint8_t* pck_buffer = new uint8_t[packet_bytes + sizeof(uint8_t)];
    pck_buffer[0] = id;
    memcpy(pck_buffer + sizeof(uint8_t), packet, packet_bytes);


    netcode_client_send_packet(client, pck_buffer, packet_bytes + sizeof(uint8_t));

    delete[] pck_buffer;
}

void network::update() {
    
    for (int i = 0; i < NETCODE_MAX_PACKET_SIZE; ++i)
        packet_data[i] = (uint8_t)i;

    while (*reinterpret_cast<DWORD*>(0xC8D4C0) != 9)
        Sleep(10);

    while (true) {
        netcode_client_update(client, time_);

        if (network::is_connected())
        {
            for (int i = 0; i < 256; i++) {
                if (id_in_map(i)) {
                   auto timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - players[i].lastOnFootSync).count() / 1000.f;

                   if (timer > 1.5f && players[i].ped) 
                       players[i].ped->SetPosn(0, 0, 0);
                   
                }
            }
            network::receive_packets();
            network::send_foot_sync();
        }

        netcode_sleep(delta_time);

        time_ += delta_time;
    }
}

void network::shutdown() {
    if (network::is_connected())
        netcode_client_disconnect(client);

    netcode_client_destroy(client);
    netcode_term();
}