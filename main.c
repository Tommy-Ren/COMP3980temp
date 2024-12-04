//
// Created by tommy on 11/25/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

void print_usage(const char *program_name)
{
    printf("Usage: %s -s -a <ip_address> [-i k|j|r] (server)\n", program_name);
    printf("       %s -c -a <ip_address> [-i k|j|r] (client)\n", program_name);
    printf("       -i: Input method (k = keyboard [default], j = joystick, r = random)\n");
}

int main(int argc, char *argv[])
{
    bool is_server = -1; // -1 indicates uninitialized
    const char *ip_address = NULL;
    char input_method = 'k';

    // Parse command-line arguments
    if (argc >= 4)
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

        // Parse optional input method
        if (argc == 6 && strcmp(argv[4], "-i") == 0)
        {
            if (argv[5][0] == 'k' || argv[5][0] == 'j' || argv[5][0] == 'r')
            {
                input_method = argv[5][0];
            }
            else
            {
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
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
        start_game(is_server, ip_address, input_method);
    }
    else
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
