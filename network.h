//
// Created by tommy on 11/25/24.
//

#ifndef NETWORK_H
#define NETWORK_H

#include "object.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 12345
#define BUFFER_SIZE (sizeof(Player) + MAX_BULLETS * sizeof(Bullet))

void init_network(int is_server, const char *ip_address);
void send_game_state(const Player *player, const Bullet *bullets);
void receive_game_state(Player *player, Bullet *bullets);
void close_network();

#endif // NETWORK_H
