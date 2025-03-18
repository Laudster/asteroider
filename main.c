#include "defines.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int last_frame_time = 0;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

int pos[2] = {500, 500};
int moveDir[2] = {0, 0};
float direction = 107.0f;
float direction2 = 67.0f;
float direction3 = 0.0f;

int accelerating = 0;

int numBullets = 0;

float *bullets;

int canShoot = 0;

int counter = 0;

const bool *keyboard_state = NULL;

float getRadius(float angle)
{
    return angle * 3.14 / 180.0f;
}

float* calculateCharacterDirection()
{
    static float result[2];

    float radius = getRadius(direction);
    float radius2 = getRadius(direction2);
    float radius3 = getRadius(direction3);

    result[0] = (cosf(radius) + cosf(radius2) + cosf(radius3) / 3.0f);
    result[1] = (sinf(radius) + sinf(radius2) + sinf(radius3) / 3.0f);

    return result;
}

void renderLine(int posX, int posY, int length, float angle)
{
    float radius = getRadius(angle);
    float endX = posX + length * cosf(radius);
    float endY = posY + length * sinf(radius);

    SDL_RenderLine(renderer, posX, posY, (int)endX, (int)endY);
}

void drawAsteroid(SDL_Renderer *renderer, int centerX, int centerY, int radius, int numPoints) {
    // Calculate angle increment
    float angleIncrement = 360.0f / numPoints;

    // Array to store vertices
    int x[numPoints], y[numPoints];

    // Calculate vertices with deterministic variation in radius
    for (int i = 0; i < numPoints; i++) {
        float angle = i * angleIncrement;
        float angleRad = angle * M_PI / 180.0f;

        // Deterministic variation using a sawtooth wave
        float variation = fmodf(angle / 150.0f, 1.0f); //Creates repeating segments
        float r = radius + radius * variation;

        x[i] = centerX + (int)(r * cosf(angleRad));
        y[i] = centerY + (int)(r * sinf(angleRad));
    }

    // Draw lines connecting the vertices
    for (int i = 0; i < numPoints; i++) {
        int nextIndex = (i + 1) % numPoints; // Wrap around to connect the last and first points
        SDL_RenderLine(renderer, x[i], y[i], x[nextIndex], y[nextIndex]);
    }
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_CreateWindowAndRenderer("Asteroider", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
        SDL_Log("Failed creating window and renderer %s \n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    bullets = (float *)malloc(sizeof(float) * 4);

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

        bullets[i * 4] += bullets[i*4 + 2];
        bullets[i * 4 + 1] += bullets[i*4 + 3];
    }

    renderLine(pos[0], pos[1], 50, direction);
    renderLine(pos[0], pos[1], 50, direction2);

    float radius3 = getRadius(direction3);
    float rotatedOffset3X = -11 * cosf(radius3) - 35 * sinf(radius3);
    float rotatedOffset3Y = -11 * sinf(radius3) + 35 * cosf(radius3);
  
    renderLine(pos[0] + rotatedOffset3X, pos[1] + rotatedOffset3Y, 25, direction3);

    if (accelerating == 1) {
        float flameOffsetX = 3;
        float flameOffsetY = 53;
        float flameAngleRad = getRadius(direction3);

        float rotatedFlameOffsetX = flameOffsetX * cosf(flameAngleRad) - flameOffsetY * sinf(flameAngleRad);
        float rotatedFlameOffsetY = flameOffsetX * sinf(flameAngleRad) + flameOffsetY * cosf(flameAngleRad);

        float flameBaseX = pos[0] + rotatedFlameOffsetX;
        float flameBaseY = pos[1] + rotatedFlameOffsetY;

        renderLine(flameBaseX, flameBaseY, 12, direction3 + 250);
        renderLine(flameBaseX, flameBaseY, 12, direction3 + 290);
    }

    SDL_RenderPresent(renderer);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    counter++;

    keyboard_state = SDL_GetKeyboardState(NULL);

    if (keyboard_state[SDL_SCANCODE_SPACE] && canShoot == 0)
    {
        float *characterDirection = calculateCharacterDirection();

        bullets = (float *)realloc(bullets, sizeof(float) * (numBullets + 1) * 4);
        bullets[numBullets * 4] = pos[0];
        bullets[numBullets * 4 + 1] = pos[1];
        bullets[numBullets * 4 + 2] = characterDirection[0] * -1 * 5;
        bullets[numBullets * 4 + 3] = characterDirection[1] * -1 * 5;

        numBullets += 1;
        canShoot = 20;
    }

    if (counter == 300){
        counter = 0;

        int newNumBullets = 0;
        float *newBullets = NULL;

        for (int i = 0; i < numBullets; i++) {
            if (bullets[i * 4] < WINDOW_WIDTH + 20 && bullets[i * 4] > -20 && bullets[i * 4 + 1] < WINDOW_HEIGHT + 20 && bullets[i * 4 +1] > -20) {
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
        float *characterDirection = calculateCharacterDirection();

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