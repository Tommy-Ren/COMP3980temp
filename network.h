//
// Created by tommy on 11/25/24.
//

#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "object.h"

#define BUFFER_SIZE (sizeof(Player) + (MAX_BULLETS * sizeof(Bullet)))
#define PORT 12345
#define ERR_NONE 0
#define ERR_NO_DIGITS 1
#define ERR_OUT_OF_RANGE 2
#define ERR_INVALID_CHARS 3

// Declare network content
struct network
{
    int sockfd;
    struct sockaddr_storage local_addr;
    struct sockaddr_storage peer_addr;
    socklen_t local_addr_len;
    socklen_t peer_addr_len;
};

// Define global network struct
static struct network ctx;

// Function to open network socket
void openNetworkSocketServer(const char *ip_address, in_port_t port, int *err);
void openNetworkSocketClient(const char *ip_address, in_port_t port, int *err);
void setupNetworkAddress(struct sockaddr_storage *addr, socklen_t *addr_len, const char *address, in_port_t port, int *err);

// Convert port if network socket as input
in_port_t convertPort(const char *str, int *err);

// Function to handle game state
void send_game_state(const Player *player, const Bullet *bullets);
void receive_game_state(Player *player, Bullet *bullets);

// Close the socket
void close_network();

#endif // NETWORK_H
