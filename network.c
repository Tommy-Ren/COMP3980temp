#include "network.h"

void openNetworkSocketServer(const char *ip_address, in_port_t port, int *err)
{
    ctx.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx.sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up local address structure
    setupNetworkAddress(&(ctx.local_addr), &(ctx.local_addr_len), ip_address, port, err);
    if (*err != 0)
    {
        close(ctx.sockfd);
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the local address
    if (bind(ctx.sockfd, (struct sockaddr *)&(ctx.local_addr), ctx.local_addr_len) < 0)
    {
        perror("Bind failed");
        close(ctx.sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Server initialized and waiting for client...\n");

    // Ensure correct address length for recvfrom
    ctx.peer_addr_len = sizeof(ctx.peer_addr);

    // Wait for the first message to get the client's address
    Player temp;
    if (recvfrom(ctx.sockfd, &temp, sizeof(temp), 0, (struct sockaddr *)&(ctx.peer_addr), &(ctx.peer_addr_len)) < 0)
    {
        perror("Failed to receive initial message");
        close(ctx.sockfd);
        exit(EXIT_FAILURE);
    }

    // Debugging the peer address
    printf("Client connected! Address: %s\n", inet_ntoa(((struct sockaddr_in *)&ctx.peer_addr)->sin_addr));
}

// Function to open client network socket
void openNetworkSocketClient(const char *ip_address, in_port_t port, int *err)
{
    ctx.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx.sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up peer address structure
    setupNetworkAddress(&(ctx.peer_addr), &(ctx.peer_addr_len), ip_address, port, err);
    if (*err != 0)
    {
        close(ctx.sockfd);
        exit(EXIT_FAILURE);
    }

    // Send an initial message to the server
    Player temp = {0, 0};
    if (sendto(ctx.sockfd, &temp, BUFFER_SIZE, 0, (struct sockaddr *)&(ctx.peer_addr), ctx.peer_addr_len) < 0)
    {
        perror("Failed to send initial message");
        close(ctx.sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server! Address: %s\n", inet_ntoa(((struct sockaddr_in *)&ctx.peer_addr)->sin_addr));
}

// Set up address structure
void setupNetworkAddress(struct sockaddr_storage *addr, socklen_t *addr_len, const char *address, in_port_t port, int *err)
{
    in_port_t net_port = htons(port);
    memset(addr, 0, sizeof(struct sockaddr_storage));
    *err = 0;

    // Try to interpret the address as IPv4
    if (inet_pton(AF_INET, address, &((struct sockaddr_in *)addr)->sin_addr) == 1)
    {
        struct sockaddr_in *ipv4_addr = (struct sockaddr_in *)addr;
        ipv4_addr->sin_family = AF_INET;
        ipv4_addr->sin_port = net_port;
        *addr_len = sizeof(struct sockaddr_in);
    }
    // If IPv4 fails, try interpreting it as IPv6
    else if (inet_pton(AF_INET6, address, &((struct sockaddr_in6 *)addr)->sin6_addr) == 1)
    {
        struct sockaddr_in6 *ipv6_addr = (struct sockaddr_in6 *)addr;
        ipv6_addr->sin6_family = AF_INET6;
        ipv6_addr->sin6_port = net_port;
        *addr_len = sizeof(struct sockaddr_in6);
    }
    // If neither IPv4 nor IPv6, set an error
    else
    {
        fprintf(stderr, "%s is not a valid IPv4 or IPv6 address\n", address);
        *err = errno;
    }
}

// Function to convert port if network socket as type
in_port_t convertPort(const char *str, int *err)
{
    in_port_t port;
    char *endptr;
    long val;

    *err = ERR_NONE;
    port = 0;
    errno = 0;
    val = strtol(str, &endptr, 10); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // Check if no digits were found
    if (endptr == str)
    {
        *err = ERR_NO_DIGITS;
        return port;
    }

    // Check for out-of-range errors
    if (val < 0 || val > UINT16_MAX)
    {
        *err = ERR_OUT_OF_RANGE;
        return port;
    }

    // Check for trailing invalid characters
    if (*endptr != '\0')
    {
        *err = ERR_INVALID_CHARS;
        return port;
    }

    port = (in_port_t)val;
    return port;
}

void send_game_state(const Player *player, const Bullet *bullets)
{
    // Calculate the size needed for player and bullet data
    size_t data_size = sizeof(Player) + MAX_BULLETS * sizeof(Bullet);

    // Ensure that BUFFER_SIZE is large enough to hold the data
    if (data_size > BUFFER_SIZE)
    {
        fprintf(stderr, "Buffer size is too small for player and bullet data!\n");
        return;
    }

    // Allocate buffer based on calculated data_size
    char *buffer = malloc(data_size);

    // Check if buffer created successfully
    if (buffer == NULL)
    {
        fprintf(stderr, "Buffer allocation failed!\n");
        return;
    }

    // Initialize the buffer with 0s
    memset(buffer, 0, data_size);

    // Copy player state to the buffer
    memcpy(buffer, player, sizeof(Player));

    // Copy bullets to the buffer after player data
    memcpy(buffer + sizeof(Player), bullets, MAX_BULLETS * sizeof(Bullet));

    // Send the buffer to the peer
    if (sendto(ctx.sockfd, buffer, data_size, 0, (struct sockaddr *)&ctx.peer_addr, ctx.peer_addr_len) < 0)
    {
        perror("Failed to send game state");
    }

    // Free the buffer after use
    free(buffer);
}

void receive_game_state(Player *player, Bullet *bullets)
{
    char *buffer = malloc(BUFFER_SIZE);

    // Check if buffer created successfully
    if (buffer == NULL)
    {
        fprintf(stderr, "Buffer allocation failed!\n");
        return;
    }
    memset(buffer, 0, BUFFER_SIZE);

    // Receive data from the peer
    if (recvfrom(ctx.sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL) < 0)
    {
        perror("Failed to receive game state");
        return;
    }

    // Extract player state from the buffer
    if (sizeof(Player) > BUFFER_SIZE)
    {
        fprintf(stderr, "Player data exceeds buffer size!\n");
        return;
    }
    memcpy(player, buffer, sizeof(Player));

    // Extract bullets from the buffer
    if (sizeof(Player) + MAX_BULLETS * sizeof(Bullet) > BUFFER_SIZE)
    {
        fprintf(stderr, "Bullets data exceeds buffer size!\n");
        return;
    }
    memcpy(bullets, buffer + sizeof(Player), MAX_BULLETS * sizeof(Bullet));

    // Free buffer
    free(buffer);
}

void close_network()
{
    close(ctx.sockfd);
}
