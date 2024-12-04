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

void start_game(bool is_server, const char *ip_address);

#endif // GAME_H
