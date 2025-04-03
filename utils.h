#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "font.h"

typedef struct {
    float x, y, angle, velX, velY;
} Line;

float getRadius(float angle)
{
    return angle * 3.14 / 180.0f;
}

float* calculateCharacterDirection(float direction, float direction2, float direction3)
{
    static float result[2];

    float radius = getRadius(direction);
    float radius2 = getRadius(direction2);
    float radius3 = getRadius(direction3);

    result[0] = (cosf(radius) + cosf(radius2) + cosf(radius3) / 3.0f);
    result[1] = (sinf(radius) + sinf(radius2) + sinf(radius3) / 3.0f);

    return result;
}

void renderLine(SDL_Renderer *renderer, int posX, int posY, int length, float angle)
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

void drawAlienShip(SDL_Renderer *renderer, int x, int y, float size) {
    SDL_RenderLine(renderer, x, y, x + (45 * size), y);
  
    SDL_RenderLine(renderer, x + (45 * size), y, x + (63 * size), y - (22 * size));
    SDL_RenderLine(
        renderer, x + (63 * size), y - (22 * size), x + (41 * size), y - (41 * size));
    SDL_RenderLine(
        renderer, x + (41 * size), y - (41 * size), x + (36 * size), y - (54 * size));
  
    SDL_RenderLine(renderer, x + (36 * size), y - (54 * size), x + (9 * size), y - (54 * size));
  
    SDL_RenderLine(renderer, x + (9 * size), y - (54 * size), x + (4 * size), y - (41 * size));
    SDL_RenderLine(
        renderer, x + (4 * size), y - (41 * size), x - (18 * size), y - (22 * size));
    SDL_RenderLine(renderer, x - (18 * size), y - (22 * size), x, y);
  
    SDL_RenderLine(
        renderer, x + (63 * size), y - (22 * size), x - (18 * size), y - (22 * size));
    SDL_RenderLine(
        renderer, x + (41 * size), y - (41 * size), x + (4 * size), y - (41 * size));
  }
  
  
float distancePointToLineSegment(int lineX1, int lineY1, int lineX2, int lineY2, int pointX, int pointY) {
    float dx = lineX2 - lineX1;
    float dy = lineY2 - lineY1;

    if (dx == 0 && dy == 0) {
        return sqrt((pointX - lineX1) * (pointX - lineX1) + (pointY - lineY1) * (pointY - lineY1));
    }

    float t = ((pointX - lineX1) * dx + (pointY - lineY1) * dy) /
    (dx * dx + dy * dy);

    t = fmax(0.0f, fmin(1.0f, t));

    float closestX = lineX1 + t * dx;
    float closestY = lineY1 + t * dy;

    return sqrt((pointX - closestX) * (pointX - closestX) + (pointY - closestY) * (pointY - closestY));
}

bool checkPlayerCircleCollision(int playerPos[2], float playerDirection1, float playerDirection2, float playerDirection3, int asteroidX, int asteroidY, int radius) {
    int line1Length = 50;
    int line2Length = 50;
    int line3Length = 25;

    float angleRad1 = playerDirection1 * M_PI / 180.0f;
    float angleRad2 = playerDirection2 * M_PI / 180.0f;
    float angleRad3 = playerDirection3 * M_PI / 180.0f;

    int line1X2 = playerPos[0] +
    (int)(line1Length * cosf(angleRad1));

    int line1Y2 = playerPos[1] +
    (int)(line1Length * sinf(angleRad1));

    int line2X2 = playerPos[0] +
    (int)(line2Length * cosf(angleRad2));

    int line2Y2 = playerPos[1] +
    (int)(line2Length * sinf(angleRad2));

    float rotatedOffset3X =
    -11 * cosf(getRadius(playerDirection3)) -
    35 * sinf(getRadius(playerDirection3));

    float rotatedOffset3Y =
    -11 * sinf(getRadius(playerDirection3)) +
    35 * cosf(getRadius(playerDirection3));

    int line3X1 = playerPos[0] + (int)rotatedOffset3X;
    int line3Y1 = playerPos[1] + (int)rotatedOffset3Y;

    int line3X2 = line3X1 +
    (int)(line3Length * cosf(angleRad3));
    int line3Y2 = line3Y1 +
    (int)(line3Length * sinf(angleRad3));

    float dist1 = distancePointToLineSegment(
    playerPos[0], playerPos[1], line1X2, line1Y2,
    asteroidX, asteroidY);

    float dist2 = distancePointToLineSegment(
    playerPos[0], playerPos[1], line2X2, line2Y2,
    asteroidX, asteroidY);

    float dist3 = distancePointToLineSegment(
    line3X1, line3Y1, line3X2, line3Y2,
    asteroidX, asteroidY);

    return (dist1 < radius ||
    dist2 < radius ||
    dist3 < radius);
}

bool checkBulletCollision(int asteroidX, int asteroidY, int radius, float bulletX, float bulletY, float bulletRadius) {
    int dx = asteroidX - bulletX;
    int dy = asteroidY - bulletY;
    int distanceSquared = dx * dx + dy * dy;
    int radiiSum = radius + bulletRadius;

    return distanceSquared <= radiiSum * radiiSum;
}

static SDL_Texture *fontTexture;

SDL_Texture* loadFont(SDL_Renderer *renderer){
    SDL_Surface *surface = SDL_CreateSurface(FONTWIDTH, FONTHEIGHT,
        SDL_GetPixelFormatForMasks(32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));

    Uint32 white = SDL_MapSurfaceRGB(surface, 255, 255, 255);
    Uint32 black = SDL_MapSurfaceRGB(surface, 0, 0, 0);

    for (int y = 0; y < FONTHEIGHT; y++) {
        for (int x = 0; x < FONTWIDTH; x++) {
            Uint32 color = font[y][x] ? white : black;
            ((Uint32 *)surface->pixels)[y * FONTWIDTH + x] = color;
        }
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surface);

    return texture;
}

void draw_char(SDL_Renderer *renderer, unsigned char character, int cx, int cy, int scale) {
    if (fontTexture == NULL) {
        fontTexture = loadFont(renderer);
    }

    SDL_Point cell = {
        (character-32) % (FONTWIDTH / GLYPHWIDTH),
        (character-32) / (FONTWIDTH / GLYPHWIDTH)
    };

    SDL_FRect srcRect = {cell.x * GLYPHWIDTH, cell.y * GLYPHHEIGHT, GLYPHWIDTH, GLYPHHEIGHT};

    SDL_FRect destRect = {cx, cy, GLYPHWIDTH * scale, GLYPHHEIGHT * scale};

    SDL_RenderTexture(renderer, fontTexture, &srcRect, &destRect);
}

void draw_text(SDL_Renderer *renderer, const char *text, int x, int y, int scale)
{
    for (size_t i = 0; text[i] != '\0'; i++) {
        draw_char(renderer, text[i], x + i * GLYPHWIDTH * scale, y, scale);
    }
}