#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdlib.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef struct { float x, y, angle, velX, velY, rotVel; } Line;
static Line lines[3];
static int dying = 0;

void initLines(int w, int h) {
  for (int i = 0; i < 3; i++) {
    lines[i].x = w / 2.0f; lines[i].y = h / 2.0f;
    lines[i].angle = rand() % (360 - 0 + 1) + 0;
    lines[i].velX = cosf(lines[i].angle) * 20;
    lines[i].velY = sinf(lines[i].angle) * 0;
  }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_CreateWindowAndRenderer("Lines", 600, 400, 0, &window, &renderer);
  initLines(600, 400); return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_QUIT)
    return SDL_APP_SUCCESS;  else return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderClear(renderer);

  if (dying) for (int i = 0; i < 3; i++) {
      lines[i].x += lines[i].velX / 60.0f;
      lines[i].y += lines[i].velY / 60.0f;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderLine(renderer, lines[i].x, lines[i].y,
                   lines[i].x + 20 * cosf(lines[i].angle),
                   lines[i].y + 20 * sinf(lines[i].angle));
  } else dying = 1;

  SDL_Delay(16);

  SDL_RenderPresent(renderer); return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
}
