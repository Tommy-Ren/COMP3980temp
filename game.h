//
// Created by tommy on 11/25/24.
//

#ifndef GAME_H
#define GAME_H

#include "object.h"
#include "network.h"
#include <ncurses.h>
#include <time.h>     // For random input
#include <SDL2/SDL.h> // For joystick handling

void start_game(bool is_server, char *ip_address, in_port_t port, char *input_method, int *err);

#endif // GAME_H
