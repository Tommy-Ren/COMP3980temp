//
// Created by tommy and Kiet on 11/25/24.
//

#include "game.h"

// Private function prototypes
static void render_screen(const Player *player1, const Player *player2, const Bullet bullets[]);
static void handle_bullets(Bullet bullets[], Player *opponent);
static void check_bullets_collide(Bullet bullets[]);
static void shoot_bullet(Bullet bullets[], const Player *shooter);
static int find_inactive_bullet(Bullet bullets[]);
static char get_random_input(const Player *player1, const Player *player2, Bullet bullets[]);
static char get_joystick_input(SDL_GameController *controller);
static void get_socket(bool is_server, char *ip_address, in_port_t port, int *err);

void start_game(bool is_server, char *ip_address, in_port_t port, char *input_method, int *err)
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
    get_socket(is_server, ip_address, port, err);

    // Initialize random seed for random input
    if (strcmp(input_method, "rd") == 0)
    {
        srand(time(NULL)); // Seed random number generator
    }

    // Initialize ncurses
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(0); // Non-blocking input

    // Initialize SDL for joystick input
    SDL_GameController *controller = NULL;
    if (strcmp(input_method, "js") == 0)
    {
        if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
        {
            endwin();
            fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
            return;
        }

        if (SDL_NumJoysticks() > 0)
        {
            controller = SDL_GameControllerOpen(0);
            if (!controller)
            {
                SDL_Quit();
                endwin();
                fprintf(stderr, "Could not open game controller: %s\n", SDL_GetError());
                return;
            }
        }
        else
        {
            SDL_Quit();
            endwin();
            fprintf(stderr, "No game controllers detected.\n");
            return;
        }
    }

    int running = ACTIVE;
    while (running)
    {
        char input = 0;
        if (strcmp(input_method, "kb") == 0)
        {
            // Get keyboard input
            input = getch();
        }
        else if (strcmp(input_method, "js") == 0)
        {
            input = get_joystick_input(controller);
        }
        else if (strcmp(input_method, "rd") == 0)
        {
            // Generate random input based on timer
            input = get_random_input(&local_player, &remote_player, bullets);
        }

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

static char get_random_input(const Player *player1, const Player *player2, Bullet bullets[])
{
    static int counter = 0;
    counter++;

    if (counter % 5 == 0)
    {
        // Check for shoot every 80 frames
        if (counter % 80 == 0)
        {
            shoot_bullet(bullets, player1); // Player 1 shoots
            shoot_bullet(bullets, player2); // Player 2 shoots
        }
        else
        {
            // Check for direction change every frames
            int random_direction = rand() % 4; // Random direction (0-3)
            switch (random_direction)
            {
            case 0:
                return 'w'; // Up
            case 1:
                return 'a'; // Left
            case 2:
                return 's'; // Down
            case 3:
                return 'd'; // Right
            }
        }
    }
    return 0; // No movement for this frame
}

static char get_joystick_input(SDL_GameController *controller)
{
    SDL_Event event;
    static int move_counter = 0;         // To handle overly sensitive joystick
    static const int move_threshold = 2; // Adjust this value for responsiveness

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_CONTROLLERAXISMOTION)
        {
            // Increment the counter for motion events
            move_counter++;
            if (move_counter < move_threshold)
            {
                return 0; // Skip overly frequent events
            }
            move_counter = 0; // Reset counter after handling

            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
            {
                if (event.caxis.value < -8000)
                    return 'w'; // Up
                if (event.caxis.value > 8000)
                    return 's'; // Down
            }
            else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
            {
                if (event.caxis.value < -8000)
                    return 'a'; // Left
                if (event.caxis.value > 8000)
                    return 'd'; // Right
            }
        }
        else if (event.type == SDL_CONTROLLERBUTTONDOWN)
        {
            if (event.cbutton.button == 0) // Button 0 (A button) for shooting
            {
                return ' '; // Space to shoot
            }
        }
    }
    return 0;
}

// Function to get which type of input are passing through command-line
static void get_socket(bool is_server, char *ip_address, in_port_t port, int *err)
{

    // Network socket server as type
    if (ip_address != NULL)
    {
        // If the user choose server type
        if (is_server == 1)
        {
            openNetworkSocketServer(ip_address, port, err);
        }

        // If the user choose client type
        else if (is_server == 0)
        {
            openNetworkSocketClient(ip_address, port, err);
        }
    }
    else
    {
        perror("IP Address is invalid");
    }
}
