//
// Created by tommy on 11/25/24.
//

#include "game.h"

// Private function prototypes
static void render_screen(const Player *player1, const Player *player2, const Bullet bullets[]);
static void handle_bullets(Bullet bullets[], Player *opponent);
static void check_bullets_collide(Bullet bullets[]);
static void shoot_bullet(Bullet bullets[], const Player *shooter);
static int find_inactive_bullet(Bullet bullets[]);

void start_game(bool is_server, const char *ip_address)
{
    // Initialize players
    Player local_player, remote_player;
    if (is_server)
    {
        init_player(&local_player, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
        init_player(&remote_player, 3 * SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
    }
    else
    {
        init_player(&local_player, 3 * SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
        init_player(&remote_player, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
    }

    // Initialize bullets
    Bullet bullets[MAX_BULLETS] = {0};
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].active = DEAD;
    }

    // Initialize network
    init_network(is_server, ip_address);

    // Initialize ncurses
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(0); // Non-blocking input

    int running = ACTIVE;
    while (running)
    {
        char input = getch();

        if (input != 0)
        {
            // Player 1 controls (WASD + Space to shoot)
            if (input == 'w' || input == 'a' || input == 's' || input == 'd' || input == 'W' || input == 'A' || input == 'S' || input == 'D')
            {
                update_player_position(&local_player, input);
                wrap_player_position(&local_player);
            }
            else if (input == ' ')
            { // Space to shoot
                shoot_bullet(bullets, &local_player);
            }
        }

        // Update bullets
        handle_bullets(bullets, &remote_player);
        handle_bullets(bullets, &local_player);
        check_bullets_collide(bullets);

        if (local_player.alive != ACTIVE || remote_player.alive != ACTIVE)
        {
            running = DEAD;
        }

        // Render the screen
        clear();
        render_screen(&local_player, &remote_player, bullets);
        refresh();

        // Send and receive game state
        send_game_state(&local_player, bullets);
        receive_game_state(&remote_player, bullets);

        // Delay for smoother gameplay
        usleep(50000);
    }

    // Cleanup
    close_network();
    endwin();

    // Check game-ending conditions
    if (local_player.alive != ACTIVE)
    {
        printf("Remote player wins!\n");
        running = 0;
    }
    else if (remote_player.alive != ACTIVE)
    {
        printf("Local player wins!\n");
        running = 0;
    }
    else
    {
        printf("Error!\n");
    }
}

static void render_screen(const Player *player1, const Player *player2, const Bullet bullets[])
{
    // Draw players
    if (player1->alive)
    {
        mvprintw(player1->y, player1->x, "M");
    }
    if (player2->alive)
    {
        mvprintw(player2->y, player2->x, "E");
    }

    // Draw bullets for Player 1
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            mvprintw(bullets[i].y, bullets[i].x, (bullets[i].direction % 2 == 0) ? "|" : "-");
        }
    }
}

static void handle_bullets(Bullet bullets[], Player *opponent)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            move_bullet(&bullets[i], opponent);
            if (is_bullet_shoot(&bullets[i], opponent))
            {
                opponent->alive = 0;   // Opponent is hit
                bullets[i].active = 0; // Deactivate the bullet
            }
        }
    }
}

static void check_bullets_collide(Bullet bullets[])
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            for (int j = 0; j < MAX_BULLETS; j++)
            {
                if (bullets[j].active && i != j)
                {
                    if (bullets[i].x == bullets[j].x && bullets[i].y == bullets[j].y)
                    {
                        bullets[i].active = 0;
                        bullets[j].active = 0;
                    }
                }
            }
        }
    }
}

static void shoot_bullet(Bullet bullets[], const Player *shooter)
{
    int bullet_index = find_inactive_bullet(bullets);
    if (bullet_index != -1)
    {
        init_bullet(&bullets[bullet_index], shooter->x, shooter->y, shooter->direction);
    }
}

static int find_inactive_bullet(Bullet bullets[])
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
        {
            return i;
        }
    }
    return -1; // No inactive bullet found
}
