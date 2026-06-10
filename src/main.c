#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game_state.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define FPS 60

int main(int argc, char *argv[]) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    bool running = true;
    Uint32 last_tick;
    const float frame_ms = 1000.0f / FPS;

    (void)argc;
    (void)argv;
    system("chcp 65001 > nul");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL 초기화 실패: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        printf("SDL_ttf 초기화 실패: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow(
        "정보통신공학과: 캡스톤 퀘스트",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("창 생성 실패: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer) {
        printf("렌더러 생성 실패: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    init_game();
    last_tick = SDL_GetTicks();

    while (running) {
        Uint32 now;
        Uint32 frame_time;
        float dt;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN &&
                       event.key.keysym.sym == SDLK_ESCAPE &&
                       current_state == STATE_MAIN_MENU) {
                running = false;
            } else {
                handle_event(&event);
            }
        }

        now = SDL_GetTicks();
        dt = (now - last_tick) / 1000.0f;
        last_tick = now;
        update_game(dt);

        SDL_SetRenderDrawColor(renderer, 16, 20, 32, 255);
        SDL_RenderClear(renderer);
        draw_game(renderer);
        SDL_RenderPresent(renderer);

        frame_time = SDL_GetTicks() - now;
        if (frame_time < frame_ms) {
            SDL_Delay((Uint32)(frame_ms - frame_time));
        }
    }

    shutdown_game();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
