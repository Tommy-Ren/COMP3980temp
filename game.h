//
// Created by tommy on 11/25/24.
//

#ifndef GAME_H
#define GAME_H

#include "object.h"
#include "network.h"
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>     // For random input
#include <SDL2/SDL.h> // For joystick handling

void start_game(bool is_server, const char *ip_address, char input_method);

#endif // GAME_H
