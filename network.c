//
// Created by tommy on 11/25/24.
//
#include "network.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Static variables for network
static int sockfd;
static struct sockaddr_in local_addr, peer_addr;
static socklen_t peer_addr_len = sizeof(peer_addr);

void init_network(int is_server, const char *ip_address)
{
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up local address
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(PORT);
    local_addr.sin_addr.s_addr = is_server ? INADDR_ANY : inet_addr(ip_address);

    if (is_server)
    {
        // Bind the socket to the local address
        if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
        {
            perror("Bind failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Server initialized and waiting for client...\n");

        // Wait for the first message to establish peer connection
        char buffer[BUFFER_SIZE];
        if (recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&peer_addr, &peer_addr_len) < 0)
        {
            perror("Failed to receive initial message");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Client connected!\n");
    }
    else
    {
        // Set up peer address for client
        memset(&peer_addr, 0, sizeof(peer_addr));
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(PORT);
        peer_addr.sin_addr.s_addr = inet_addr(ip_address);

        // Send an initial message to the server
        char buffer[BUFFER_SIZE] = {0};
        if (sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&peer_addr, peer_addr_len) < 0)
        {
            perror("Failed to send initial message");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Connected to server!\n");
    }
}

void send_game_state(const Player *player, const Bullet *bullets)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Copy player state to the buffer
    memcpy(buffer, player, sizeof(Player));

    // Copy bullets to the buffer
    memcpy(buffer + sizeof(Player), bullets, MAX_BULLETS * sizeof(Bullet));

    // Send the buffer to the peer
    if (sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&peer_addr, peer_addr_len) < 0)
    {
        perror("Failed to send game state");
    }
}

void receive_game_state(Player *player, Bullet *bullets)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Receive data from the peer
    if (recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL) < 0)
    {
        perror("Failed to receive game state");
        return;
    }

    // Extract player state from the buffer
    memcpy(player, buffer, sizeof(Player));

    // Extract bullets from the buffer
    memcpy(bullets, buffer + sizeof(Player), MAX_BULLETS * sizeof(Bullet));
}

void close_network()
{
    close(sockfd);
}
