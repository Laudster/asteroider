#include "defines.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

int last_frame_time = 0;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

int pos[2] = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
int moveDir[2] = {0, 0};
float direction = 107.0f;
float direction2 = 67.0f;
float direction3 = 0.0f;

int accelerating = 0;

int numBullets = 0;
float *bullets;

int numAsteroids = 0;
int *asteroids;

int numAliens = 0;
int *aliens;

int numAlienBullets = 0;
float *alienBullets;

int canShoot = 0;

int counter = 0;

float delta_time;

const bool *keyboard_state = NULL;

void die() {
    pos[0] = WINDOW_WIDTH / 2;
    pos[1] = WINDOW_HEIGHT / 2;
    moveDir[0] = 0;
    moveDir[1] = 0;
    direction = 107.0f;
    direction2 = 67.0f;
    direction3 = 0.0f;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_CreateWindowAndRenderer("Asteroider", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
        SDL_Log("Failed creating window and renderer %s \n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    bullets = (float *)malloc(sizeof(float) * 4);
    asteroids = (int *)malloc(sizeof(int) * 5);
    aliens = (int *)malloc(sizeof(int) * 6);
    alienBullets = (float *)malloc(sizeof(float) * 4);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.scancode == SDL_SCANCODE_Q || event->key.scancode == SDL_SCANCODE_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
    }
    return SDL_APP_CONTINUE;
}

void Draw() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < numBullets; i++) {
        SDL_FRect bullet = {bullets[i * 4], bullets[i * 4 + 1], 5, 5};
        SDL_RenderRect(renderer, &bullet);

        for (int a = 0; a < numAsteroids; a++) {
            if (checkBulletCollision(asteroids[a * 5], asteroids[a * 5 + 1], asteroids[a * 5 + 4], bullets[i * 4], bullets[i * 4 + 1], 5)) {
                bullets[i * 4] = 99999;

                asteroids[a * 5 + 4] *= 0.6;

                if (asteroids[a * 5 + 4] <= 10) {
                    asteroids[a * 5] = 9999;
                }
            }
        }

        bullets[i * 4] += bullets[i*4 + 2] * delta_time * 80;
        bullets[i * 4 + 1] += bullets[i*4 + 3] * delta_time * 80;
    }

    for (int i = 0; i < numAsteroids; i++) {
        drawAsteroid(renderer, asteroids[i * 5], asteroids[i * 5 + 1], asteroids[i * 5 + 4], 10);

        asteroids[i * 5] += asteroids[i * 5  + 2] * delta_time * 50;
        asteroids[i * 5 + 1] += asteroids[i * 5 + 3] * delta_time * 50;

        if (checkPlayerCircleCollision(pos, direction, direction2, direction3, asteroids[i * 5], asteroids[i * 5 + 1], asteroids[i * 5 + 4])) {
            die();
        }
    }

    for (int i = 0; i < numAliens; i++) {
        drawAlienShip(renderer, aliens[i * 6], aliens[i * 6 + 1], (float)aliens[i * 6 + 3] / 10);

        if (checkPlayerCircleCollision(pos, direction, direction2, direction3, aliens[i * 6], aliens[i * 6 + 1], (float)aliens[i * 6 + 3] * 4)) {
            die();
        }

        for (int a = 0; a < numAsteroids; a++) {
            if (checkBulletCollision(asteroids[a * 5], asteroids[a * 5 + 1], asteroids[a * 5 + 4], aliens[i * 6], aliens[i * 6 + 1], (float)aliens[i * 6 + 3] * 4)) {
                aliens[i * 6] = 99999;
            }
        }

        for (int b = 0; b < numBullets; b++) {
            if (checkBulletCollision(bullets[b * 4], bullets[b * 4 + 1], 5, aliens[i * 6], aliens[i * 6 + 1], (float)aliens[i * 6 + 3] * 4)) {
                aliens[i * 6] = 99999;
                bullets[b * 4] = -999;
            }
        }

        aliens[i * 6] += 150 * delta_time * aliens[i * 6 + 2];

        if (aliens[i * 6 + 5] == 0 && aliens[i * 6] > 0 && aliens[i * 6] < WINDOW_WIDTH)  {
            aliens[i * 6 + 5] = 40;

            alienBullets = realloc(alienBullets, sizeof(float) * (numAlienBullets + 1) * 4);
            alienBullets[numAlienBullets * 4] = aliens[i * 6];
            alienBullets[numAlienBullets * 4 + 1] = aliens[i * 6 + 1];

            if (rand() % (1 - 0 + 1) - 0) {
                alienBullets[numAlienBullets * 4 + 2] = rand() % (5 - 3 + 1) + 3;
            } else {
                alienBullets[numAlienBullets * 4 + 2] = rand() % (-5 - -3 + 1) + -3;
            }

            alienBullets[numAlienBullets * 4  + 3] = rand() % (1 - -1 + 1) + -1;

            numAlienBullets += 1;
        }

        aliens[i * 6 + 5] -= 1;

        if (rand() % (200 - 0 + 1) - 0 == 0) {
            if (rand() % (1 - 0 + 1) - 0) {
                aliens[i * 6 + 4] = 20;
            } else {
                aliens[i * 6 + 4] = -20;
            }
        }

        if (aliens[i * 6 + 4] != 0) {
            if (aliens[i * 6 + 4] > 0) {
                aliens[i * 6 + 1] += 200 * delta_time;
                aliens[i * 6 + 4] -= 1;
            } else {
                aliens[i * 6 + 1] += -200 * delta_time;
                aliens[i * 6 + 4] += 1;
            }
        }

        if (aliens[i * 6 + 1] > WINDOW_HEIGHT) {
            aliens[i * 6 + 1] = 0;
        }

        if (aliens[i * 6 + 1] < 0) {
            aliens[i * 6 + 1] = WINDOW_HEIGHT;
        }
    }

    for (int i = 0; i < numAlienBullets; i++) {
        SDL_FRect alienBullet = {alienBullets[i * 4], alienBullets[i *  4 + 1], 5,  5};
        SDL_RenderRect(renderer, &alienBullet);

        if (checkPlayerCircleCollision(pos, direction, direction2, direction3, alienBullets[i * 4], alienBullets[i * 4 + 1], 5)) {
            die();
            alienBullets[i * 4] = 2300;
        }

        for (int a = 0; a < numAsteroids; a++) {
            if (checkBulletCollision(asteroids[a * 5], asteroids[a * 5 + 1], asteroids[a * 5 + 4], alienBullets[i * 4], alienBullets[i * 4 + 1], 5)) {
                alienBullets[i * 4] = 9999;

                asteroids[a * 5 + 4] *= 0.6;

                if (asteroids[a * 5 + 4] <= 10) {
                    asteroids[a * 5] = 9999;
                }
            }
        }

        alienBullets[i * 4] += alienBullets[i * 4 + 2] * 100 * delta_time;
        alienBullets[i * 4 + 1] += 500 * alienBullets[i * 4 + 3] * delta_time;
    }

    renderLine(renderer, pos[0], pos[1], 50, direction);
    renderLine(renderer, pos[0], pos[1], 50, direction2);

    float radius3 = getRadius(direction3);
    float rotatedOffset3X = -11 * cosf(radius3) - 35 * sinf(radius3);
    float rotatedOffset3Y = -11 * sinf(radius3) + 35 * cosf(radius3);
  
    renderLine(renderer, pos[0] + rotatedOffset3X, pos[1] + rotatedOffset3Y, 25, direction3);

    if (accelerating == 1) {
        float flameOffsetX = 3;
        float flameOffsetY = 53;
        float flameAngleRad = getRadius(direction3);

        float rotatedFlameOffsetX = flameOffsetX * cosf(flameAngleRad) - flameOffsetY * sinf(flameAngleRad);
        float rotatedFlameOffsetY = flameOffsetX * sinf(flameAngleRad) + flameOffsetY * cosf(flameAngleRad);

        float flameBaseX = pos[0] + rotatedFlameOffsetX;
        float flameBaseY = pos[1] + rotatedFlameOffsetY;

        renderLine(renderer, flameBaseX, flameBaseY, 12, direction3 + 250);
        renderLine(renderer, flameBaseX, flameBaseY, 12, direction3 + 290);
    }

    SDL_RenderPresent(renderer);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    counter++;

    keyboard_state = SDL_GetKeyboardState(NULL);

    if (keyboard_state[SDL_SCANCODE_SPACE] && canShoot == 0)
    {
        float *characterDirection = calculateCharacterDirection(direction, direction2, direction3);

        bullets = (float *)realloc(bullets, sizeof(float) * (numBullets + 1) * 4);
        bullets[numBullets * 4] = pos[0];
        bullets[numBullets * 4 + 1] = pos[1];
        bullets[numBullets * 4 + 2] = characterDirection[0] * -1 * 5;
        bullets[numBullets * 4 + 3] = characterDirection[1] * -1 * 5;

        numBullets += 1;
        canShoot = 30;
    }

    if (counter == 100) {
        if (rand() % (2 - 0 + 1) + 0 == 0) {
            aliens = (int *)realloc(aliens, sizeof(int) * (numAliens + 1) * 6);
            
            
            if (rand() % (1 - 0 + 1) - 0 == 0) {
                aliens[numAliens * 6] = 0;
                aliens[numAliens * 6 + 1] = rand() % (WINDOW_HEIGHT - 0 + 1) - 0;
                aliens[numAliens * 6 + 2] = 1;
            } else {
                aliens[numAliens * 6] = WINDOW_WIDTH;
                aliens[numAliens * 6 + 1] = rand() % (WINDOW_HEIGHT - 0 + 1) - 0;
                aliens[numAliens * 6 + 2] = -1;
            }

            aliens[numAliens * 6 + 3] = rand() % (12 - 5 + 1) + 5;
            aliens[numAliens * 6 + 4] = 0;
            aliens[numAliens * 6 + 5] = 40;

            numAliens += 1;
        }
    }


    if (counter % 150 == 0) {
        int x;
        int y;

        int pos = rand() % (3 - 0 + 1) - 0;

        switch (pos)
        {
        case 0:
            x = 0;
            y = rand() % (WINDOW_HEIGHT - 0 + 1) - 0;
            break;

        case 1:
            x = WINDOW_WIDTH;
            y = rand() % (WINDOW_HEIGHT - 0 + 1) - 0;
            break;

        case 2:
            x = rand() % (WINDOW_WIDTH - 0 + 1) - 0;
            y = 0;
            break;

        case 3:
            x = rand() % (WINDOW_WIDTH - 0 + 1) - 0;
            y = WINDOW_HEIGHT;
            break;
        }

        int moveX = (WINDOW_WIDTH - x  - x )* 0.003;
        int moveY = (WINDOW_HEIGHT - y  - y) * 0.003;

        asteroids = (int *)realloc(asteroids, sizeof(int) * (numAsteroids + 1) * 5);
        asteroids[numAsteroids * 5] = x;
        asteroids[numAsteroids * 5 + 1] = y;
        asteroids[numAsteroids * 5 + 2] = moveX;
        asteroids[numAsteroids * 5 + 3] = moveY;
        asteroids[numAsteroids * 5 + 4] = rand() % (40 - 10 + 1) + 10;

        numAsteroids += 1;
    }

    if (counter == 50) {
        int newNumAliens = 0;
        int *newAliens = NULL;

        for (int i = 0; i < numAliens; i++) {
            if (aliens[i * 6] < WINDOW_WIDTH + 20 && aliens[i * 6] > -20) {
                newAliens = (int *)realloc(newAliens, sizeof(int) * (newNumAliens + 1) * 6);

                newAliens[newNumAliens * 6] = aliens[i * 6];
                newAliens[newNumAliens * 6 + 1] = aliens[i * 6 + 1];
                newAliens[newNumAliens * 6 + 2] = aliens[i * 6 + 2];
                newAliens[newNumAliens * 6 + 3] = aliens[i * 6 + 3];
                newAliens[newNumAliens * 6 + 4] = aliens[i * 6 + 4];
                newAliens[newNumAliens * 6 + 5] = aliens[i * 6 + 5];

                newNumAliens += 1;
            }
        }

        free(aliens);

        aliens = newAliens;
        numAliens = newNumAliens;
    }

    if (counter == 150) {

        int newNumAsteroids = 0;
        int *newAsteroids = NULL;

        for (int i = 0; i < numAsteroids; i++) {
            if (asteroids[i * 5] < WINDOW_WIDTH + 20 && asteroids[i * 5] > -20) {
                newAsteroids = (int *)realloc(newAsteroids, sizeof(int) * (newNumAsteroids + 1) * 5);

                newAsteroids[newNumAsteroids * 5] = asteroids[i * 5];
                newAsteroids[newNumAsteroids * 5 + 1] = asteroids[i * 5 + 1];
                newAsteroids[newNumAsteroids * 5 + 2] = asteroids[i * 5 + 2];
                newAsteroids[newNumAsteroids * 5 + 3] = asteroids[i * 5 + 3];
                newAsteroids[newNumAsteroids * 5 + 4] = asteroids[i * 5 + 4];

                newNumAsteroids += 1;
            }
        }

        free(asteroids);

        asteroids = newAsteroids;
        numAsteroids = newNumAsteroids;
    }

    if (counter == 250) {
        int newNumAlienBullets = 0;
        float *newAlienBullets = NULL;

        for (int i = 0; i < numAlienBullets; i++) {
            if (alienBullets[i * 4] < WINDOW_WIDTH + 20 && alienBullets[i * 4] > -20) {
                newAlienBullets = (float *)realloc(newAlienBullets, sizeof(float) * (newNumAlienBullets + 1) * 4);
                
                newAlienBullets[newNumAlienBullets * 4] = alienBullets[i * 4];
                newAlienBullets[newNumAlienBullets * 4 + 1] = alienBullets[i * 4 + 1];
                newAlienBullets[newNumAlienBullets * 4 + 2] = alienBullets[i * 4 + 2];
                newAlienBullets[newNumAlienBullets * 4 + 3] = alienBullets[i * 4 + 3];

                newNumAlienBullets += 1;
            }
        }

        free(alienBullets);

        alienBullets = newAlienBullets;
        numAlienBullets = newNumAlienBullets;
    }

    if (counter == 300){
        counter = 0;

        int newNumBullets = 0;
        float *newBullets = NULL;

        for (int i = 0; i < numBullets; i++) {
            if (bullets[i * 4] < WINDOW_WIDTH + 20 && bullets[i * 4] > -20) {
                newBullets = (float *)realloc(newBullets, sizeof(float) * (newNumBullets + 1) * 4);

                newBullets[newNumBullets * 4] = bullets[i * 4];
                newBullets[newNumBullets * 4 + 1] = bullets[i * 4 + 1];
                newBullets[newNumBullets * 4 + 2] = bullets[i * 4 + 2];
                newBullets[newNumBullets * 4 + 3] = bullets[i * 4 + 3];

                newNumBullets += 1;
            }
        }

        free(bullets);

        bullets = newBullets;
        numBullets = newNumBullets;
    }

    if (canShoot > 0) canShoot -= 1;

    accelerating = 0;

    if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {        
        float *characterDirection = calculateCharacterDirection(direction, direction2, direction3);

        moveDir[0] += (SPEED * characterDirection[0] * 0.04);
        moveDir[1] += (SPEED * characterDirection[1] * 0.04);

        accelerating = 1;
    }

    if (keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT]) {
        direction += 3;
        direction2 += 3;
        direction3 += 3;
    }

    if (keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT]) {
        direction -= 3;
        direction2 -= 3;
        direction3 -= 3;
    }

    if (moveDir[0] != 0 || moveDir[1] != 0) {
        pos[0] += moveDir[0] * delta_time * -1;
        pos[1] += moveDir[1] * delta_time * -1;

        if (moveDir[0] > 150 || moveDir[0] < -150 || moveDir[1] > 150 || moveDir[1] < -150) {
            moveDir[0] *= 0.98;
            moveDir[1] *= 0.98;
        }
    }

    if (pos[0] > WINDOW_WIDTH) {
        pos[0] = 0;
    }

    if (pos[1] > WINDOW_HEIGHT) {
        pos[1] = 0;
    }

    if (pos[0] < 0) {
        pos[0] = WINDOW_WIDTH;
    }

    if (pos[1] < 0) {
        pos[1] = WINDOW_HEIGHT;
    }

    Draw();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    free(bullets);
}