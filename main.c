#include "defines.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

int last_frame_time = 0;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

int pos[2] = {500, 500};
float direction = 107.0f;
float direction2 = 67.0f;
float direction3 = 0.0f;

float velocity = 0.0f;

const bool *keyboard_state = NULL;

float getRadius(float angle)
{
    return angle * 3.14 / 180.0f;
}

void renderLine(int posX, int posY, int length, float angle)
{
    float radius = getRadius(angle);
    float endX = posX + length * cosf(radius);
    float endY = posY + length * sinf(radius);

    SDL_RenderLine(renderer, posX, posY, (int)endX, (int)endY);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_CreateWindowAndRenderer("Asteroider", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
        SDL_Log("Failed creating window and renderer %s \n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_KEY_DOWN) {
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

    renderLine(pos[0], pos[1], 50, direction);
    renderLine(pos[0], pos[1], 50, direction2);

    float radius3 = getRadius(direction3);
    float rotatedOffset3X = -11 * cosf(radius3) - 35 * sinf(radius3);
    float rotatedOffset3Y = -11 * sinf(radius3) + 35 * cosf(radius3);
  
    renderLine(pos[0] + rotatedOffset3X, pos[1] + rotatedOffset3Y, 25, direction3);


    if (velocity >= 0.9) {
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

    keyboard_state = SDL_GetKeyboardState(NULL);

    if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {        
        velocity = 1.0f;
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

    if (velocity != 0)
    {
        float radius = getRadius(direction);
        float radius2 = getRadius(direction2);
        float radius3 = getRadius(direction3);

        float offsetX = (cosf(radius) + cosf(radius2) + cosf(radius3) / 3.0f);
        float offsetY = (sinf(radius) + sinf(radius2) + sinf(radius3) / 3.0f);

        pos[0] -= (SPEED * delta_time) * offsetX * velocity;
        pos[1] -= (SPEED * delta_time) * offsetY * velocity;

        if (velocity > 0.25) velocity *= 0.99f;
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
}