#include <SDL.h>
#include <emscripten.h>
#include <cmath>
#include <iostream>


SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
int height = 640;
int width  = 640;


const int MAP_H = 8;
const int MAP_W = 7;

const int UNIT_H = height / MAP_H;
const int UNIT_W = width / MAP_W;

int map[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,2,0,1,0,1},
    {1,0,2,0,2,0,1},
    {1,2,0,2,0,0,1},
    {1,0,2,0,0,0,1},
    {1,1,1,1,1,1,1},
};

void render2D_map() {
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    for (int i = 0; i < MAP_H; i++) {
        SDL_RenderDrawLine(
            renderer,
            0,
            i * (height / MAP_H),
            width,
            i * (height / MAP_H)
        );
    }

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    for (int i = 0; i < MAP_W; i++) {
        SDL_RenderDrawLine(
            renderer,
            i * (width / MAP_W),
            0,
            i * (width / MAP_W),
            height
        );
    }

    for (int r = 0; r < MAP_H; r++) {
        for (int c = 0; c < MAP_W; c++) {
            if (map[r][c] == 0) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_Rect rect {
                    c * (width / MAP_W),
                    r * (height / MAP_H),
                    (width / MAP_W),
                    (height / MAP_H)
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void loop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {}

    SDL_SetRenderDrawColor(renderer, 140, 0, 0, 255);
    SDL_RenderClear(renderer);

    render2D_map();

    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Raycast Prototype",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_SOFTWARE
    );

    emscripten_set_main_loop(loop, 0, true);
    return 0;
}
