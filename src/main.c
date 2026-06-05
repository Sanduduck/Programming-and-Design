#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#include "game_state.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define FPS 60

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    if (TTF_Init() < 0) {
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Survice ICE",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // init_game() 이후에도 current_state는 STATE_MAIN_MENU라서
    // 프로그램을 켜면 가장 먼저 메뉴 화면이 나온다.
    init_game();

    bool running = true;
    SDL_Event event;
    Uint32 last_tick = SDL_GetTicks();
    const float frame_ms = 1000.0f / FPS;

    while (running) {
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

        Uint32 now = SDL_GetTicks();
        float dt = (now - last_tick) / 1000.0f;
        last_tick = now;

        update_game(dt);

        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderClear(renderer);
        // draw_game()이 현재 상태를 보고 메뉴 화면 또는 플레이 화면을 그린다.
        draw_game(renderer);
        SDL_RenderPresent(renderer);

        Uint32 frame_time = SDL_GetTicks() - now;
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
