#include "network.h"

static uint8_t private_key[NETCODE_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea,
                                                  0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4,
                                                  0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                  0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 }; 
double time_ = 0.0;
double delta_time = 1.0 / 60.0;

static volatile int quit = 0;

std::map<int, Player> Players;
bool id_in_map(int id) {
    return !(Players.find(id) == Players.end());
}

void interrupt_handler(int signal)
{
    (void)signal;
    quit = 1;
}

float get_distance(Position pos1, Position pos2)
{
    const auto x_dif = pos1.x - pos2.x;
    const auto y_dif = pos1.y - pos2.y;
    const auto z_dif = pos1.z - pos2.z;

    return sqrt(x_dif * x_dif + y_dif * y_dif + z_dif * z_dif);
}

struct netcode_server_t* server = nullptr;

void network::init() {
    if (netcode_init() != NETCODE_OK)
    {
        printf("error: failed to initialize netcode\n");
        return;
    }

    netcode_log_level(NETCODE_LOG_LEVEL_INFO);

    const char* server_address = "5.13.169.35:40000";

    struct netcode_server_config_t server_config;
    netcode_default_server_config(&server_config);
    server_config.protocol_id = TEST_PROTOCOL_ID;

    memcpy(&server_config.private_key, private_key, NETCODE_KEY_BYTES);

    server = netcode_server_create(server_address, &server_config, time_);

    if (!server)
    {
        printf("error: failed to create server\n");
        return;
    }

    netcode_server_start(server, NETCODE_MAX_CLIENTS);

    signal(SIGINT, interrupt_handler);
}

void network::handle_packet(void* packet, int client_author) {
    const auto packet_data = (uint8_t*)packet;
    const PacketID id = (PacketID)packet_data[0];

    switch (id)
    {
    case SPAWN_PLAYER:
    {
        if (id_in_map(client_author))
            return;

        const pCreateLocalPlayer* data = reinterpret_cast<pCreateLocalPlayer*>(packet_data + sizeof(uint8_t));
        
        printf("spawn player: %s, %d, %f, %f, %f\n", (const char*)data->name, data->modelIndex, data->pos.x, data->pos.y, data->pos.z);

        Player player;
        player.id = client_author;
        player.modelIndex = data->modelIndex;
        player.pos = data->pos;
        strcpy_s(player.name, data->name);
        
        memcpy(&Players[client_author], &player, sizeof(player));
        std::cout << Players[client_author].name << std::endl;
        send_packet_except(client_author, SPAWN_PLAYER, &player, sizeof(Player));
        break;
    }
    case FOOT_SYNC: 
    {
        if (!id_in_map(client_author))
            return;

        OnFootSync data = *reinterpret_cast<OnFootSync*>(packet_data + sizeof(uint8_t));
        Players[client_author].pos = data.pos; 
        data.id = client_author;
        data.modelIndex = Players[client_author].modelIndex;
        strcpy_s(data.name, Players[client_author].name);
        send_packet_nearby(client_author, FOOT_SYNC, &data, sizeof(OnFootSync));
        break;
    }
    case CHAT_MESSAGE: 
    {
        ChatMessage data = *reinterpret_cast<ChatMessage*>(packet_data + sizeof(uint8_t));
        data.id = client_author;
        strcpy_s(data.name, Players[client_author].name);
        send_packet_everyone(CHAT_MESSAGE, &data, sizeof(ChatMessage));
        break;
    }
    }
}   

void network::send_packet_nearby(int client, int id, void* packet, int packet_bytes) {
    for (int client_id = 0; client_id < NETCODE_MAX_CLIENTS; client_id++) {


        if (client_id == client)
            continue;

        if (!netcode_server_client_connected(server, client_id))
            continue;

        if (!id_in_map(client_id) || !id_in_map(client))
            continue;

        auto dist = get_distance(Players[client_id].pos, Players[client].pos);

        if (dist >= 150.f)
            continue;

        send_packet(client_id, id, packet, packet_bytes);
    }
}

void network::send_packet_everyone(int id, void* packet, int packet_bytes) {
    for (int client_id = 0; client_id < NETCODE_MAX_CLIENTS; client_id++) {
        if (!netcode_server_client_connected(server, client_id))
            continue;

        send_packet(client_id, id, packet, packet_bytes);
    }
}

void network::send_packet_except(int client, int id, void* packet, int packet_bytes) {
    for (int client_id = 0; client_id < NETCODE_MAX_CLIENTS; client_id++) {
        if (client_id == client)
            continue;

        if (!netcode_server_client_connected(server, client_id))
            continue;

        send_packet(client_id, id, packet, packet_bytes);
    }
}

void network::send_packet(int client, int id, void* packet, int packet_bytes) {
    uint8_t* pck_buffer = new uint8_t[packet_bytes + sizeof(uint8_t)];
    pck_buffer[0] = id;
    memcpy(pck_buffer + sizeof(uint8_t), packet, packet_bytes);
    //printf("%d\n", id);
    netcode_server_send_packet(server, client, pck_buffer, packet_bytes + sizeof(uint8_t));

    delete[] pck_buffer;
}

void network::update() {
    while (!quit)
    {
        netcode_server_update(server, time_);

        for (int client_index = 0; client_index < NETCODE_MAX_CLIENTS; ++client_index)
        {
            while (1)
            {
                if (id_in_map(client_index) && !netcode_server_client_connected(server, client_index))
                {
                    Players.erase(client_index);
                    send_packet_except(client_index, DELETE_PLAYER, &client_index, sizeof(int));
                    printf("deleted player %d\n", client_index);
                }

                int packet_bytes;
                uint64_t packet_sequence;
                
                void* packet = netcode_server_receive_packet(server, client_index, &packet_bytes, &packet_sequence);
                
                if (!packet)
                    break;

                handle_packet(packet, client_index);

                netcode_server_free_packet(server, packet);
            }
        }

        netcode_sleep(delta_time);

        time_ += delta_time;
    }
}

void network::shutdown() {
    netcode_server_destroy(server);
    netcode_term();
}