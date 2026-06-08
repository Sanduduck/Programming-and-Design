// 정통에서 살아남기 - main.c
// SDL2 초기화 및 메인 루프

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "game_state.h"

#define FPS 60

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // 콘솔 한글 출력 (UTF-8 코드페이지)
    system("chcp 65001 > nul");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    TTF_Init();

    SDL_Window *window = SDL_CreateWindow(
        "정통에서 살아남기",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    // 콘솔 안내
    printf("=== 정통에서 살아남기 ===\n");
    printf("[메뉴] 1: 시작, 2: 랭킹, 3: 설정 (마우스 클릭도 가능), ESC: 종료\n");
    printf("[게임] 좌/우 또는 A/D: 이동, Space 또는 W: 점프(2단)\n");

    // 게임 초기화
    init_game();

    // 메인 루프
    bool running = true;
    SDL_Event event;
    Uint32 last_tick = SDL_GetTicks();
    const float frame_ms = 1000.0f / FPS;

    while (running) {
        // 이벤트 처리
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN &&
                       event.key.keysym.sym == SDLK_ESCAPE &&
                       current_state == STATE_MAIN_MENU) {
                // 메뉴에서 ESC만 종료
                running = false;
            } else {
                handle_event(&event);
            }
        }

        // 시간 갱신
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_tick) / 1000.0f;
        last_tick = now;

        // 업데이트/렌더
        update_game(dt);

        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderClear(renderer);
        draw_game(renderer);
        SDL_RenderPresent(renderer);

        // 프레임 제한
        Uint32 frame_time = SDL_GetTicks() - now;
        if (frame_time < frame_ms) {
            SDL_Delay((Uint32)(frame_ms - frame_time));
        }
    }

    // 정리
    shutdown_game();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
