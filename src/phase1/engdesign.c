// 공학설계입문 — 홈씨어터 빛 회피. X자 4섹터(0:상/1:우/2:하/3:좌).
#include "engdesign.h"
#include "../player.h"
#include <SDL_ttf.h>

#define PROJ_X         640
#define PROJ_Y         560

#define PLAT_X         580
#define PLAT_Y         380
#define PLAT_W         120
#define PLAT_H         10

#define WARN_DUR       2.0f
#define ACTIVE_DUR     0.6f

#define INTRO_TEXT_DUR 2.0f
#define INTRO_TEXT     "빛으로 가라"

static const struct { float t; int sector; } schedule[] = {
    {  0.3f, 2 }, {  3.0f, 0 }, {  5.7f, 1 },
    {  8.4f, 3 }, { 11.1f, 2 }, { 13.8f, 0 },
};
#define SCHEDULE_LEN ((int)(sizeof(schedule)/sizeof(schedule[0])))

static float     pattern_time;
static int       next_spawn;
static int       beam_sector;
static float     beam_age;
static bool      beam_active;
static TTF_Font *intro_font = NULL;

static void load_intro_font(void) {
    if (intro_font) return;
    intro_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 72);
    if (!intro_font) intro_font = TTF_OpenFont("C:/Windows/Fonts/gulim.ttc", 72);
}

void engdesign_start(void) {
    pattern_time = 0;
    next_spawn   = 0;
    beam_active  = false;
    load_intro_font();
}

static int sector_of(float px, float py) {
    float s1 = px * WINDOW_H - py * WINDOW_W;
    float s2 = px * WINDOW_H + py * WINDOW_W - (float)WINDOW_W * WINDOW_H;
    if (s1 > 0) return (s2 < 0) ? 0 : 1;
    return            (s2 < 0) ? 3 : 2;
}

void engdesign_update(float dt) {
    pattern_time += dt;

    if (!beam_active && next_spawn < SCHEDULE_LEN &&
        schedule[next_spawn].t <= pattern_time) {
        beam_sector = schedule[next_spawn].sector;
        beam_age    = 0;
        beam_active = true;
        next_spawn++;
    }

    if (beam_active) {
        beam_age += dt;
        if (beam_age >= WARN_DUR && beam_age < WARN_DUR + ACTIVE_DUR) {
            float corners[4][2] = {
                { player.x,            player.y            },
                { player.x + PLAYER_W, player.y            },
                { player.x,            player.y + PLAYER_H },
                { player.x + PLAYER_W, player.y + PLAYER_H },
            };
            bool in_light = false;
            for (int k = 0; k < 4 && !in_light; k++)
                in_light = sector_of(corners[k][0], corners[k][1]) == beam_sector;
            if (!in_light) player_damage();
        }
        if (beam_age >= WARN_DUR + ACTIVE_DUR) {
            beam_active = false;
            // 마지막 빔 종료 → 다음 패턴 진입 전에 플레이어를 화면 중앙·바닥으로
            if (next_spawn >= SCHEDULE_LEN) {
                player.x = (float)((WINDOW_W - PLAYER_W) / 2);
                player.y = (float)(FLOOR_Y - PLAYER_H);
                player.vy = 0;
                player.on_ground = true;
                player.jump_count = 0;
            }
        }
    }

    // 1방향 발판: 떨어지는 중 발 중심점이 발판 위면 스냅
    if (player.vy >= 0.0f) {
        float feet = player.y + PLAYER_H;
        float cx   = player.x + PLAYER_W / 2;
        if (cx >= PLAT_X && cx < PLAT_X + PLAT_W &&
            feet >= PLAT_Y && feet <= PLAT_Y + PLAT_H) {
            player.y = PLAT_Y - PLAYER_H;
            player.vy = 0;
            player.on_ground = true;
            player.jump_count = 0;
        }
    }
}

bool engdesign_finished(void) {
    return next_spawn >= SCHEDULE_LEN && !beam_active;
}

static void fill_sector(SDL_Renderer *r, int sector) {
    int cy = WINDOW_H / 2;
    for (int y = 0; y < FLOOR_Y; y++) {
        int d1 = y * WINDOW_W / WINDOW_H;
        int d2 = WINDOW_W - d1;
        int x1, x2;
        switch (sector) {
            case 0: if (y >= cy) continue; x1 = d1; x2 = d2; break;
            case 1: x1 = (d1 > d2) ? d1 : d2; x2 = WINDOW_W; break;
            case 2: if (y <  cy) continue; x1 = d2; x2 = d1; break;
            case 3: x1 = 0; x2 = (d1 < d2) ? d1 : d2;        break;
            default: continue;
        }
        if (x1 < x2) SDL_RenderDrawLine(r, x1, y, x2, y);
    }
}

void engdesign_draw(SDL_Renderer *r) {
    if (beam_active) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        if (beam_age < WARN_DUR) {
            int a = (int)(60.0f + 60.0f * (beam_age / WARN_DUR));
            SDL_SetRenderDrawColor(r, 240, 220, 80, (Uint8)a);
        } else {
            SDL_SetRenderDrawColor(r, 220, 240, 255, 180);
        }
        fill_sector(r, beam_sector);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

    // X자 대각선
    SDL_SetRenderDrawColor(r, 0, 80, 100, 255);
    int xf = FLOOR_Y * WINDOW_W / WINDOW_H;
    SDL_RenderDrawLine(r, 0,        0, xf,            FLOOR_Y);
    SDL_RenderDrawLine(r, WINDOW_W, 0, WINDOW_W - xf, FLOOR_Y);

    // 발판
    SDL_SetRenderDrawColor(r, 80, 80, 90, 255);
    SDL_Rect plat = { PLAT_X, PLAT_Y, PLAT_W, PLAT_H };
    SDL_RenderFillRect(r, &plat);
    SDL_SetRenderDrawColor(r, 0, 200, 220, 255);
    SDL_RenderDrawRect(r, &plat);

    // 홈씨어터
    int sx[2] = { PROJ_X - 90, PROJ_X + 90 };
    for (int i = 0; i < 2; i++) {
        SDL_SetRenderDrawColor(r, 80, 80, 90, 255);
        SDL_Rect box  = { sx[i] - 15, FLOOR_Y - 50, 30, 50 };
        SDL_RenderFillRect(r, &box);
        SDL_SetRenderDrawColor(r, 30, 30, 35, 255);
        SDL_Rect woof = { sx[i] - 10, FLOOR_Y - 30, 20, 20 };
        SDL_Rect twee = { sx[i] - 7,  FLOOR_Y - 45, 14, 10 };
        SDL_RenderFillRect(r, &woof);
        SDL_RenderFillRect(r, &twee);
    }
    SDL_SetRenderDrawColor(r, 100, 100, 110, 255);
    SDL_Rect body = { PROJ_X - 45, PROJ_Y, 90, FLOOR_Y - PROJ_Y };
    SDL_RenderFillRect(r, &body);
    SDL_SetRenderDrawColor(r, 200, 220, 255, 255);
    SDL_Rect lens = { PROJ_X - 12, PROJ_Y - 4, 24, 8 };
    SDL_RenderFillRect(r, &lens);

    // 인트로 텍스트: 첫 INTRO_TEXT_DUR초 동안 흰색으로 고정 표시
    if (pattern_time < INTRO_TEXT_DUR && intro_font) {
        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Surface *surf = TTF_RenderUTF8_Blended(intro_font, INTRO_TEXT, white);
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
            SDL_Rect dst = {
                (WINDOW_W - surf->w) / 2,
                WINDOW_H / 3 - surf->h / 2,
                surf->w, surf->h
            };
            SDL_RenderCopy(r, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }
    }
}
