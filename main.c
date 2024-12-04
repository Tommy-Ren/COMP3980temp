//
// Created by tommy on 11/25/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

void print_usage(const char *program_name)
{
    printf("Usage: %s -s -a <ip_address> (server)\n", program_name);
    printf("       %s -c -a <ip_address> (client)\n", program_name);
}

int main(int argc, char *argv[])
{
    bool is_server = -1; // -1 indicates uninitialized
    const char *ip_address = NULL;

    // Parse command-line arguments
    if (argc == 4)
    {
        if (strcmp(argv[1], "-s") == 0)
        {
            is_server = 1;
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            is_server = 0;
        }
        else
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        if (strcmp(argv[2], "-a") == 0)
        {
            ip_address = argv[3];
        }
        else
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    else
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Start the game
    if (is_server == 1 || is_server == 0)
    {
        start_game(is_server, ip_address);
    }
    else
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
