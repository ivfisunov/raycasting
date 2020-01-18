#include <stdio.h>
#include <SDL2/SDL.h>
#include <elf.h>
#include <zconf.h>

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
uint32_t *colorBuffer = NULL;
SDL_Texture *colorBufferTexture = NULL;

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

struct Ray {
    float rayAngle;
    float wallHitX;
    float wallHitY;
    float distance;
    int wasHitVertical;
    int isRayFacingUp;
    int isRayFacingDown;
    int isRayFacingLeft;
    int isRayFacingRight;
    int wallHitContent;
} rays[NUM_RAYS];

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

void castAllRays();

void castRay(float rayAngle, int stripId);

void clearColorBuffer();

void renderColorBuffer();

void generate3DProjection();

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
    free(colorBuffer);
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
    player.walkSpeed = 150; // in pixels
    player.turnSpeed = (float) (90.0 * (PI / 180)); // radians

    colorBuffer = malloc(sizeof(Uint32) * (Uint32) WINDOW_WIDTH * (Uint32) WINDOW_HEIGHT);
    colorBufferTexture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            WINDOW_WIDTH,
            WINDOW_HEIGHT
    );
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
            if (event.key.keysym.sym == SDLK_LALT && event.key.keysym.sym == SDLK_LEFT) {
                //
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

    castAllRays();
}

void castAllRays() {
    // start first ray subtracting half of our FOV
    float rayAngle = player.rotatingAngle - (FOV_ANGLE / 2);

    for (int stripId = 0; stripId < NUM_RAYS; stripId++) {
        castRay(rayAngle, stripId);
        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

float normalizeAngle(float angle) {
    angle = remainder(angle, TWO_PI);
    if (angle < 0) {
        angle = TWO_PI + angle;
    }
    return angle;
}

float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void castRay(float rayAngle, int stripId) {
    rayAngle = normalizeAngle(rayAngle);

    int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
    int isRayFacingUp = !isRayFacingDown;

    int isRayFacingRight = rayAngle < PI / 2 || rayAngle >  PI * 3 / 2;
    int isRayFacingLeft = !isRayFacingRight;

    float xintercept, yintercept;
    float xstep, ystep;

    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    int foundHorzWallHit = FALSE;
    float horzWallHitX = 0;
    float horzWallHitY = 0;
    int horzWallContent = 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += isRayFacingDown ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= isRayFacingUp ? -1 : 1;

    xstep = TILE_SIZE / tan(rayAngle);
    xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
    xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundHorzWallHit = TRUE;
            break;
        } else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }

    ///////////////////////////////////////////
    // VERTICAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    int foundVertWallHit = FALSE;
    float vertWallHitX = 0;
    float vertWallHitY = 0;
    int vertWallContent = 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += isRayFacingRight ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isRayFacingLeft ? -1 : 1;

    ystep = TILE_SIZE * tan(rayAngle);
    ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
    ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
        float yToCheck = nextVertTouchY;

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            vertWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundVertWallHit = TRUE;
            break;
        } else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horzHitDistance = foundHorzWallHit
                            ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
                            : INT_MAX;
    float vertHitDistance = foundVertWallHit
                            ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
                            : INT_MAX;

    if (vertHitDistance < horzHitDistance) {
        rays[stripId].distance = vertHitDistance;
        rays[stripId].wallHitX = vertWallHitX;
        rays[stripId].wallHitY = vertWallHitY;
        rays[stripId].wallHitContent = vertWallContent;
        rays[stripId].wasHitVertical = TRUE;
    } else {
        rays[stripId].distance = horzHitDistance;
        rays[stripId].wallHitX = horzWallHitX;
        rays[stripId].wallHitY = horzWallHitY;
        rays[stripId].wallHitContent = horzWallContent;
        rays[stripId].wasHitVertical = FALSE;
    }
    rays[stripId].rayAngle = rayAngle;
    rays[stripId].isRayFacingDown = isRayFacingDown;
    rays[stripId].isRayFacingUp = isRayFacingUp;
    rays[stripId].isRayFacingLeft = isRayFacingLeft;
    rays[stripId].isRayFacingRight = isRayFacingRight;
}

void renderRays() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < NUM_RAYS; i++) {
        SDL_RenderDrawLine(
                renderer,
                MINI_MAP_SCALE_FACTOR * player.x,
                MINI_MAP_SCALE_FACTOR * player.y,
                MINI_MAP_SCALE_FACTOR * rays[i].wallHitX,
                MINI_MAP_SCALE_FACTOR * rays[i].wallHitY
        );
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    generate3DProjection();
    renderColorBuffer();
    clearColorBuffer(0xFF000000);

    // render minimap
    renderMap();
    renderRays();
    renderPlayer();

    SDL_RenderPresent(renderer);
}

void generate3DProjection() {
    for (int i = 0; i < NUM_RAYS; ++i) {
        float normDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotatingAngle);
        float distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
        float projectedWallHeight = (TILE_SIZE / normDistance) * distanceProjPlane;

        int wallStripHeight = projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        for (int c = 0; c < wallTopPixel; ++c) {
            colorBuffer[WINDOW_WIDTH * c + i] = 0xFFb7def5;
        }
        for (int c = wallBottomPixel; c < WINDOW_HEIGHT; ++c) {
            colorBuffer[WINDOW_WIDTH * c + i] = 0xFF555555;
        }
        // rendering the walls
        for (int y = wallTopPixel; y < wallBottomPixel; ++y) {
            colorBuffer[WINDOW_WIDTH * y + i] = rays[i].wasHitVertical ? 0xFFFFFFFF: 0xFFCCCCCC;
        }
    }

}

void renderColorBuffer() {
    SDL_UpdateTexture(
            colorBufferTexture,
            NULL,
            colorBuffer,
            sizeof(Uint32) * WINDOW_WIDTH
    );
    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void clearColorBuffer(Uint32 color) {
    for (int y = 0; y < WINDOW_HEIGHT; ++y) {
        for (int x = 0; x < WINDOW_WIDTH; ++x) {
            colorBuffer[WINDOW_WIDTH * y + x] = color;
        }
    }
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

