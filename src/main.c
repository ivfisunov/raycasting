#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
//#include <math.h>

#include "constants.h"

/* GLOBAL VARIABLES */
const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int isGameRunnig = FALSE;
int ticksLastFrame;
struct Player {
    float x;
    float y;
    float width;
    float height;
    int turnDirection; // -1 for left, +1 for right
    int walkDirection; // -1 for back, +1 for front
    float rotatingAngle;
    float walkSpeed;
    float turnSpeed;
} player;

int initializeWindow();

void setup();

void processInput();

void update();

void render();

void destroyWindow();

void renderMap();

void renderPlayer();

void movePlayer(float time);

int mapHasWallAt(float x, float y);

int main(void) {
    printf("Program is running...\n");

    isGameRunnig = initializeWindow();
    setup();

    while (isGameRunnig) {
        processInput();
        update();
        render();
    }

    destroyWindow();
    return 0;
}

void destroyWindow() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int initializeWindow() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL\n");
        return FALSE;
    }

    window = SDL_CreateWindow(
            "3D WORLD",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Error creating a window\n");
        return FALSE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating a renderer\n");
        return FALSE;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return TRUE;
}

void setup() {
    player.x = (float) WINDOW_WIDTH / 2;
    player.y = (float) WINDOW_HEIGHT / 2;
    player.width = 8;
    player.height = 8;
    player.turnDirection = 0;
    player.walkDirection = 0;
    player.rotatingAngle = PI / 2;
    player.walkSpeed = 100; // in pixels
    player.turnSpeed = (float) (180.0 * (PI / 180)); // radians
}

void processInput() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT: {
            isGameRunnig = FALSE;
            break;
        }
        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                isGameRunnig = FALSE;
            }
            if (event.key.keysym.sym == SDLK_UP) {
                player.walkDirection = +1;
            }
            if (event.key.keysym.sym == SDLK_DOWN) {
                player.walkDirection = -1;
            }
            if (event.key.keysym.sym == SDLK_RIGHT) {
                player.turnDirection = +1;
            }
            if (event.key.keysym.sym == SDLK_LEFT) {
                player.turnDirection = -1;
            }
            break;
        }
        case SDL_KEYUP: {
            if (event.key.keysym.sym == SDLK_UP) {
                player.walkDirection = 0;
            }
            if (event.key.keysym.sym == SDLK_DOWN) {
                player.walkDirection = 0;
            }
            if (event.key.keysym.sym == SDLK_RIGHT) {
                player.turnDirection = 0;
            }
            if (event.key.keysym.sym == SDLK_LEFT) {
                player.turnDirection = 0;
            }
            break;
        }
    }
}

void update() {
    while (SDL_GetTicks() < (ticksLastFrame + FRAME_TIME_LENGTH)) {}

    float deltaTime = (float) (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
    ticksLastFrame = SDL_GetTicks();

    movePlayer(deltaTime);
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    renderMap();
//    renderRays();
    renderPlayer();

    SDL_RenderPresent(renderer);
}

void movePlayer(float deltaTime) {
    player.rotatingAngle += player.turnDirection * player.turnSpeed * deltaTime;
    float moveStep = player.walkDirection * player.walkSpeed * deltaTime;

    float newPlayerX = player.x + cos(player.rotatingAngle) * moveStep;
    float newPlayerY = player.y + sin(player.rotatingAngle) * moveStep;

    // TODO: perform a wall collision
    if (!mapHasWallAt(newPlayerX, newPlayerY)) {
        player.x = newPlayerX;
        player.y = newPlayerY;
    }

}

int mapHasWallAt(float x, float y) {
    if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
        return TRUE;
    }
    int mapIndexX = floor(x / TILE_SIZE);
    int mapIndexY = floor(y / TILE_SIZE);
    return map[mapIndexY][mapIndexX] != 0;
}

void renderPlayer() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect playerRect = {
            (player.x - player.width / 2) * MINI_MAP_SCALE_FACTOR,
            (player.y - player.height / 2) * MINI_MAP_SCALE_FACTOR,
            player.width * MINI_MAP_SCALE_FACTOR,
            player.height * MINI_MAP_SCALE_FACTOR
    };
    SDL_RenderFillRect(renderer, &playerRect);
    SDL_RenderDrawLine(
            renderer,
            MINI_MAP_SCALE_FACTOR * player.x,
            MINI_MAP_SCALE_FACTOR * player.y,
            MINI_MAP_SCALE_FACTOR * player.x + cos(player.rotatingAngle) * 40,
            MINI_MAP_SCALE_FACTOR * player.y + sin(player.rotatingAngle) * 40
    );
}

void renderMap() {
    for (int i = 0; i < MAP_NUM_ROWS; ++i) {
        for (int j = 0; j < MAP_NUM_COLS; ++j) {
            int tileX = j * TILE_SIZE;
            int tileY = i * TILE_SIZE;
            int tileColor = map[i][j] != 0 ? 255 : 0;

            SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);
            SDL_Rect mapTileRect = {
                    tileX * MINI_MAP_SCALE_FACTOR,
                    tileY * MINI_MAP_SCALE_FACTOR,
                    TILE_SIZE * MINI_MAP_SCALE_FACTOR,
                    TILE_SIZE * MINI_MAP_SCALE_FACTOR
            };
            SDL_RenderFillRect(renderer, &mapTileRect);
        }
    }
}

