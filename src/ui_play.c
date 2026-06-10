#define _CRT_SECURE_NO_WARNINGS
#include <SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game_state.h"
#include "player.h"
#include "score.h"
#include "ui_play.h"

#define WINDOW_W 1280
#define WINDOW_H 720
#define MAX_HP 100
#define ACTION_COUNT 4
#define BULLET_COUNT 48

#define BOX_X 300
#define BOX_Y 315
#define BOX_W 680
#define BOX_H 215

typedef enum {
    PHASE_COMMAND,
    PHASE_ITEM_SELECT,
    PHASE_EFFECT,
    PHASE_DODGE
} BattlePhase;

typedef enum {
    EFFECT_NONE,
    EFFECT_ATTACK,
    EFFECT_FIREWALL,
    EFFECT_HEAL
} EffectType;

typedef struct {
    const char *name;
    int max_hp;
    int hp;
    int level;
} Enemy;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int size;
    int active;
} Bullet;

typedef struct {
    const char *name;
    int heal;
    int count;
} HealItem;

static Enemy enemies[3];
static Bullet bullets[BULLET_COUNT];
static HealItem heal_items[3];
static BattlePhase phase;
static EffectType effect_type;
static int enemy_index;
static int selected_action;
static int selected_item;
static int debugger_count;
static int firewall_ready;
static int won;
static int effect_damage;
static int enemy_defeated;
static float dodge_timer;
static float spawn_timer;
static float hit_cooldown;
static float effect_timer;
static char message[160];
static TTF_Font *title_font;
static TTF_Font *body_font;
static TTF_Font *small_font;

static void load_fonts(void) {
    if (!title_font) title_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 32);
    if (!body_font) body_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 24);
    if (!small_font) small_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 18);
}

static void draw_text(SDL_Renderer *r, TTF_Font *font, const char *text,
                      int x, int y, SDL_Color color) {
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect dst;
    if (!font || !text) return;
    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;
    texture = SDL_CreateTextureFromSurface(r, surface);
    if (texture) {
        dst = (SDL_Rect){ x, y, surface->w, surface->h };
        SDL_RenderCopy(r, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

static void draw_outline(SDL_Renderer *r, SDL_Rect rect, int thickness,
                         Uint8 red, Uint8 green, Uint8 blue) {
    int i;
    SDL_SetRenderDrawColor(r, red, green, blue, 255);
    for (i = 0; i < thickness; i++) {
        SDL_Rect line = { rect.x + i, rect.y + i, rect.w - i * 2, rect.h - i * 2 };
        SDL_RenderDrawRect(r, &line);
    }
}

static void draw_hp_bar(SDL_Renderer *r, int x, int y, int value, int maximum) {
    SDL_Rect back = { x, y, 230, 18 };
    SDL_Rect fill = { x + 2, y + 2, (226 * value) / maximum, 14 };
    SDL_SetRenderDrawColor(r, 130, 20, 20, 255);
    SDL_RenderFillRect(r, &back);
    SDL_SetRenderDrawColor(r, 255, 220, 35, 255);
    SDL_RenderFillRect(r, &fill);
}

static int random_between(int min, int max) {
    return min + rand() % (max - min + 1);
}

static Enemy *current_enemy(void) {
    return &enemies[enemy_index];
}

static void clear_bullets(void) {
    int i;
    for (i = 0; i < BULLET_COUNT; i++) bullets[i].active = 0;
}

static void finish_battle(int victory) {
    won = victory;
    change_state(STATE_RESULT);
}

static void next_enemy(void) {
    Enemy *enemy = current_enemy();
    add_score(250 + enemy_index * 120 + player.hp);
    if (enemy_index == 2) {
        finish_battle(1);
        return;
    }
    enemy_index++;
    player.hp += 20;
    if (player.hp > MAX_HP) player.hp = MAX_HP;
    snprintf(message, sizeof(message), "* %s 해결. 다음 과제가 접속했다.", enemy->name);
    phase = PHASE_COMMAND;
}

static int remaining_heal_items(void) {
    int i;
    int total = 0;
    for (i = 0; i < 3; i++) total += heal_items[i].count;
    return total;
}

static void begin_dodge(void) {
    phase = PHASE_DODGE;
    dodge_timer = 4.5f + enemy_index * 0.8f;
    spawn_timer = 0.0f;
    player.x = BOX_X + BOX_W / 2.0f;
    player.y = BOX_Y + BOX_H / 2.0f;
    clear_bullets();
    strcpy(message, "* 방향키 또는 WASD로 패킷을 피하세요.");
}

static void begin_effect(EffectType type, int damage) {
    effect_type = type;
    effect_damage = damage;
    effect_timer = 1.15f;
    phase = PHASE_EFFECT;
}

static void finish_effect(void) {
    effect_type = EFFECT_NONE;
    if (enemy_defeated) {
        enemy_defeated = 0;
        next_enemy();
    } else {
        begin_dodge();
    }
}

static void use_heal_item(int index) {
    HealItem *item;
    if (index < 0 || index >= 3) return;
    item = &heal_items[index];
    selected_item = index;
    if (item->count <= 0) {
        snprintf(message, sizeof(message), "* %s은(는) 남아 있지 않다.", item->name);
        return;
    }
    item->count--;
    player.hp += item->heal;
    if (player.hp > MAX_HP) player.hp = MAX_HP;
    snprintf(message, sizeof(message), "* %s 사용. HP가 %d 회복되었다.", item->name, item->heal);
    begin_effect(EFFECT_HEAL, 0);
}

static void perform_action(int action) {
    Enemy *enemy = current_enemy();
    int damage = 0;
    selected_action = action;

    if (action == 0) {
        damage = random_between(18, 28);
        snprintf(message, sizeof(message), "* 패킷 송신! 내가 과제를 공격했다. %d DAMAGE.", damage);
        begin_effect(EFFECT_ATTACK, damage);
    } else if (action == 1) {
        firewall_ready = 1;
        strcpy(message, "* 방어 성공! 방화벽이 다음 피격 피해를 절반으로 줄인다.");
        begin_effect(EFFECT_FIREWALL, 0);
    } else if (action == 2) {
        if (debugger_count <= 0) {
            strcpy(message, "* 남은 디버거 사용 횟수가 없다.");
            return;
        }
        debugger_count--;
        damage = random_between(38, 50);
        snprintf(message, sizeof(message), "* 디버거 공격! 치명적 오류로 %d DAMAGE.", damage);
        begin_effect(EFFECT_ATTACK, damage);
    } else {
        if (remaining_heal_items() <= 0) {
            strcpy(message, "* 남은 회복 아이템이 없다.");
            return;
        }
        selected_item = 0;
        phase = PHASE_ITEM_SELECT;
        strcpy(message, "* 사용할 회복 아이템을 선택하세요.");
        return;
    }

    enemy->hp -= damage;
    if (enemy->hp <= 0) {
        enemy->hp = 0;
        enemy_defeated = 1;
    }
}

static void spawn_bullet(void) {
    int i;
    Enemy *enemy = current_enemy();
    for (i = 0; i < BULLET_COUNT; i++) {
        Bullet *b = &bullets[i];
        float speed;
        float target_x;
        float target_y;
        float length;
        int side;
        if (b->active) continue;

        side = rand() % 4;
        if (side == 0) {
            b->x = (float)random_between(BOX_X + 10, BOX_X + BOX_W - 10);
            b->y = (float)BOX_Y;
        } else if (side == 1) {
            b->x = (float)random_between(BOX_X + 10, BOX_X + BOX_W - 10);
            b->y = (float)(BOX_Y + BOX_H);
        } else if (side == 2) {
            b->x = (float)BOX_X;
            b->y = (float)random_between(BOX_Y + 10, BOX_Y + BOX_H - 10);
        } else {
            b->x = (float)(BOX_X + BOX_W);
            b->y = (float)random_between(BOX_Y + 10, BOX_Y + BOX_H - 10);
        }

        target_x = player.x + random_between(-45, 45);
        target_y = player.y + random_between(-30, 30);
        length = sqrtf((target_x - b->x) * (target_x - b->x) +
                       (target_y - b->y) * (target_y - b->y));
        speed = 180.0f + enemy->level * 45.0f;
        b->vx = (target_x - b->x) / length * speed;
        b->vy = (target_y - b->y) / length * speed;
        b->size = 8 + enemy->level * 2;
        b->active = 1;
        return;
    }
}

static int bullet_hits_player(const Bullet *b) {
    return b->x + b->size >= player.x - 9 &&
           b->x <= player.x + 9 &&
           b->y + b->size >= player.y - 9 &&
           b->y <= player.y + 9;
}

static void update_dodge(float dt) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    float speed = 280.0f;
    int i;

    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) player.x -= speed * dt;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) player.x += speed * dt;
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) player.y -= speed * dt;
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) player.y += speed * dt;

    if (player.x < BOX_X + 14) player.x = BOX_X + 14;
    if (player.x > BOX_X + BOX_W - 14) player.x = BOX_X + BOX_W - 14;
    if (player.y < BOX_Y + 14) player.y = BOX_Y + 14;
    if (player.y > BOX_Y + BOX_H - 14) player.y = BOX_Y + BOX_H - 14;

    spawn_timer -= dt;
    if (spawn_timer <= 0.0f) {
        spawn_bullet();
        if (enemy_index >= 1) spawn_bullet();
        spawn_timer = 0.34f - enemy_index * 0.06f;
    }
    if (hit_cooldown > 0.0f) hit_cooldown -= dt;

    for (i = 0; i < BULLET_COUNT; i++) {
        Bullet *b = &bullets[i];
        int damage;
        if (!b->active) continue;
        b->x += b->vx * dt;
        b->y += b->vy * dt;
        if (b->x < BOX_X - 30 || b->x > BOX_X + BOX_W + 30 ||
            b->y < BOX_Y - 30 || b->y > BOX_Y + BOX_H + 30) {
            b->active = 0;
            continue;
        }
        if (hit_cooldown <= 0.0f && bullet_hits_player(b)) {
            damage = 7 + enemy_index * 2;
            if (firewall_ready) {
                damage /= 2;
                firewall_ready = 0;
            }
            player.hp -= damage;
            if (player.hp < 0) player.hp = 0;
            hit_cooldown = 0.7f;
            b->active = 0;
            if (player.hp <= 0) {
                finish_battle(0);
                return;
            }
        }
    }

    dodge_timer -= dt;
    if (dodge_timer <= 0.0f) {
        phase = PHASE_COMMAND;
        clear_bullets();
        strcpy(message, "* 공격이 끝났다. 다음 명령을 선택하세요.");
    }
}

static void update_effect(float dt) {
    effect_timer -= dt;
    if (effect_timer <= 0.0f) finish_effect();
}

static void draw_boss(SDL_Renderer *r) {
    SDL_Rect monitor = { 550, 70, 180, 120 };
    SDL_Rect screen = { 565, 85, 150, 82 };
    SDL_Rect stand = { 625, 190, 30, 35 };
    SDL_Rect base = { 585, 220, 110, 12 };
    int eye_offset = ((int)(SDL_GetTicks() / 350) % 2) * 4;

    SDL_SetRenderDrawColor(r, 245, 245, 245, 255);
    SDL_RenderFillRect(r, &monitor);
    SDL_RenderFillRect(r, &stand);
    SDL_RenderFillRect(r, &base);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderFillRect(r, &screen);
    SDL_SetRenderDrawColor(r, 80, 255, 150, 255);
    SDL_RenderDrawLine(r, 580, 105, 610 + eye_offset, 105);
    SDL_RenderDrawLine(r, 670 - eye_offset, 105, 700, 105);
    SDL_RenderDrawLine(r, 595, 145, 685, 145);
    SDL_RenderDrawLine(r, 610, 155, 670, 155);
}

static void draw_heart(SDL_Renderer *r) {
    SDL_Rect left = { (int)player.x - 9, (int)player.y - 7, 9, 9 };
    SDL_Rect right = { (int)player.x, (int)player.y - 7, 9, 9 };
    SDL_Rect middle = { (int)player.x - 9, (int)player.y, 18, 9 };
    SDL_Rect tip = { (int)player.x - 4, (int)player.y + 9, 9, 7 };
    SDL_SetRenderDrawColor(r, 255, 30, 45, 255);
    SDL_RenderFillRect(r, &left);
    SDL_RenderFillRect(r, &right);
    SDL_RenderFillRect(r, &middle);
    SDL_RenderFillRect(r, &tip);
}

static void draw_action_effect(SDL_Renderer *r) {
    float progress = 1.0f - effect_timer / 1.15f;
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color yellow = { 255, 230, 40, 255 };
    SDL_Color cyan = { 50, 220, 255, 255 };
    char buffer[64];

    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    if (effect_type == EFFECT_ATTACK) {
        int packet_x = BOX_X + 65 + (int)(progress * (BOX_W - 130));
        SDL_Rect packet = { packet_x, BOX_Y + 92, 34, 20 };
        SDL_SetRenderDrawColor(r, cyan.r, cyan.g, cyan.b, 255);
        SDL_RenderFillRect(r, &packet);
        SDL_RenderDrawLine(r, packet.x - 45, packet.y + 5, packet.x - 8, packet.y + 5);
        SDL_RenderDrawLine(r, packet.x - 35, packet.y + 15, packet.x - 8, packet.y + 15);
        if (progress > 0.72f) {
            SDL_RenderDrawLine(r, BOX_X + BOX_W - 100, BOX_Y + 55,
                               BOX_X + BOX_W - 45, BOX_Y + 145);
            SDL_RenderDrawLine(r, BOX_X + BOX_W - 45, BOX_Y + 55,
                               BOX_X + BOX_W - 100, BOX_Y + 145);
            snprintf(buffer, sizeof(buffer), "-%d", effect_damage);
            draw_text(r, title_font, buffer, BOX_X + BOX_W - 150, BOX_Y + 25, yellow);
        }
        draw_text(r, body_font, "PACKET ATTACK!", BOX_X + 235, BOX_Y + 25, white);
    } else if (effect_type == EFFECT_FIREWALL) {
        SDL_Rect shield = { BOX_X + BOX_W / 2 - 75, BOX_Y + 38, 150, 135 };
        int pulse = ((int)(effect_timer * 12) % 2) * 8;
        draw_outline(r, shield, 5 + pulse / 4, cyan.r, cyan.g, cyan.b);
        SDL_RenderDrawLine(r, shield.x, shield.y, shield.x + shield.w / 2, shield.y + shield.h);
        SDL_RenderDrawLine(r, shield.x + shield.w, shield.y,
                           shield.x + shield.w / 2, shield.y + shield.h);
        draw_text(r, body_font, "FIREWALL DEFENSE!", BOX_X + 205, BOX_Y + 10, white);
    } else if (effect_type == EFFECT_HEAL) {
        int rise = (int)(progress * 55);
        draw_heart(r);
        draw_text(r, title_font, "+", (int)player.x + 25, (int)player.y - rise, yellow);
        draw_text(r, body_font, "ITEM USED - HP RECOVERED", BOX_X + 175, BOX_Y + 25, white);
    }
}

void init_battle(void) {
    srand((unsigned int)time(NULL));
    enemies[0] = (Enemy){ "SPAGHETTI CODE", 75, 75, 1 };
    enemies[1] = (Enemy){ "PACKET STORM", 110, 110, 2 };
    enemies[2] = (Enemy){ "CAPSTONE DEADLINE", 155, 155, 3 };
    enemy_index = 0;
    selected_action = 0;
    selected_item = 0;
    debugger_count = 2;
    heal_items[0] = (HealItem){ "캔커피", 20, 2 };
    heal_items[1] = (HealItem){ "에너지 드링크", 35, 1 };
    heal_items[2] = (HealItem){ "컵라면", 50, 1 };
    firewall_ready = 0;
    won = 0;
    effect_type = EFFECT_NONE;
    effect_damage = 0;
    enemy_defeated = 0;
    dodge_timer = 0.0f;
    spawn_timer = 0.0f;
    hit_cooldown = 0.0f;
    effect_timer = 0.0f;
    phase = PHASE_COMMAND;
    clear_bullets();
    strcpy(message, "* 정보통신 과제가 길을 막았다.");
}

void handle_play_event(SDL_Event *e) {
    int clicked = -1;
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int i;
        if (phase == PHASE_COMMAND) {
            for (i = 0; i < 4; i++) {
                SDL_Rect button = { 45 + i * 305, 625, 270, 58 };
                if (e->button.x >= button.x && e->button.x < button.x + button.w &&
                    e->button.y >= button.y && e->button.y < button.y + button.h) {
                    clicked = i;
                    break;
                }
            }
        } else if (phase == PHASE_ITEM_SELECT) {
            for (i = 0; i < 3; i++) {
                SDL_Rect item = { BOX_X + 40, BOX_Y + 60 + i * 40, 580, 36 };
                if (e->button.x >= item.x && e->button.x < item.x + item.w &&
                    e->button.y >= item.y && e->button.y < item.y + item.h) {
                    clicked = i;
                    break;
                }
            }
        }
        if (clicked >= 0 && clicked < (phase == PHASE_COMMAND ? 4 : 3)) {
            if (phase == PHASE_COMMAND) perform_action(clicked);
            else use_heal_item(clicked);
        }
        return;
    }
    if (e->type != SDL_KEYDOWN || e->key.repeat != 0) return;

    if (e->key.keysym.sym == SDLK_m || e->key.keysym.sym == SDLK_ESCAPE) {
        if (phase == PHASE_ITEM_SELECT) {
            phase = PHASE_COMMAND;
            strcpy(message, "* 아이템 선택을 취소했다.");
        } else {
            change_state(STATE_MAIN_MENU);
        }
        return;
    }
    if (phase == PHASE_ITEM_SELECT) {
        if (e->key.keysym.sym == SDLK_UP)
            selected_item = (selected_item + 2) % 3;
        else if (e->key.keysym.sym == SDLK_DOWN)
            selected_item = (selected_item + 1) % 3;
        else if (e->key.keysym.sym >= SDLK_1 && e->key.keysym.sym <= SDLK_3)
            use_heal_item((int)(e->key.keysym.sym - SDLK_1));
        else if (e->key.keysym.sym == SDLK_RETURN || e->key.keysym.sym == SDLK_SPACE)
            use_heal_item(selected_item);
        return;
    }
    if (phase != PHASE_COMMAND) return;

    if (e->key.keysym.sym == SDLK_LEFT)
        selected_action = (selected_action + ACTION_COUNT - 1) % ACTION_COUNT;
    else if (e->key.keysym.sym == SDLK_RIGHT)
        selected_action = (selected_action + 1) % ACTION_COUNT;
    else if (e->key.keysym.sym == SDLK_1) perform_action(0);
    else if (e->key.keysym.sym == SDLK_2) perform_action(1);
    else if (e->key.keysym.sym == SDLK_3) perform_action(2);
    else if (e->key.keysym.sym == SDLK_4) perform_action(3);
    else if (e->key.keysym.sym == SDLK_RETURN || e->key.keysym.sym == SDLK_SPACE)
        perform_action(selected_action);
}

void update_play(float dt) {
    if (phase == PHASE_DODGE) update_dodge(dt);
    else if (phase == PHASE_EFFECT) update_effect(dt);
}

void draw_play(SDL_Renderer *r) {
    Enemy *enemy = current_enemy();
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color gray = { 175, 175, 175, 255 };
    SDL_Color orange = { 255, 150, 20, 255 };
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_Rect battle_box = { BOX_X, BOX_Y, BOX_W, BOX_H };
    const char *actions[ACTION_COUNT] = {
        "1 SEND", "2 FIREWALL", "3 DEBUG", "4 DRINK"
    };
    char buffer[120];
    int i;

    load_fonts();
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderFillRect(r, &bg);
    draw_boss(r);

    draw_text(r, body_font, enemy->name, 40, 35, white);
    snprintf(buffer, sizeof(buffer), "TASK %d / 3", enemy_index + 1);
    draw_text(r, small_font, buffer, 1090, 42, gray);
    draw_hp_bar(r, 520, 255, enemy->hp, enemy->max_hp);

    draw_outline(r, battle_box, 5, 255, 255, 255);
    if (phase == PHASE_COMMAND) {
        draw_text(r, body_font, message, BOX_X + 30, BOX_Y + 35, white);
        snprintf(buffer, sizeof(buffer), "* DEBUG %d회 / ITEM %d개",
                 debugger_count, remaining_heal_items());
        draw_text(r, small_font, buffer, BOX_X + 30, BOX_Y + 85, gray);
        draw_text(r, small_font, "* 좌우 이동, Enter 선택", BOX_X + 30, BOX_Y + 135, gray);
    } else if (phase == PHASE_ITEM_SELECT) {
        draw_text(r, body_font, "* 회복 아이템 선택", BOX_X + 30, BOX_Y + 20, white);
        for (i = 0; i < 3; i++) {
            SDL_Color color = i == selected_item ? (SDL_Color){255, 230, 40, 255} : white;
            snprintf(buffer, sizeof(buffer), "%d. %-18s +%d HP   x%d",
                     i + 1, heal_items[i].name, heal_items[i].heal, heal_items[i].count);
            draw_text(r, body_font, buffer, BOX_X + 55, BOX_Y + 65 + i * 40, color);
        }
        draw_text(r, small_font, "* 위/아래, Enter 선택 / ESC 취소",
                  BOX_X + 360, BOX_Y + 180, gray);
    } else if (phase == PHASE_EFFECT) {
        draw_action_effect(r);
        draw_text(r, small_font, message, BOX_X + 30, BOX_Y + 180, white);
    } else {
        for (i = 0; i < BULLET_COUNT; i++) {
            SDL_Rect bullet;
            if (!bullets[i].active) continue;
            bullet = (SDL_Rect){
                (int)bullets[i].x, (int)bullets[i].y,
                bullets[i].size, bullets[i].size
            };
            SDL_SetRenderDrawColor(r, 245, 245, 245, 255);
            SDL_RenderFillRect(r, &bullet);
        }
        if (hit_cooldown <= 0.0f || ((int)(hit_cooldown * 12) % 2) == 0)
            draw_heart(r);
        snprintf(buffer, sizeof(buffer), "%.1f", dodge_timer);
        draw_text(r, small_font, buffer, BOX_X + BOX_W - 50, BOX_Y + 10, gray);
    }

    draw_text(r, body_font, "STUDENT", 70, 565, white);
    draw_text(r, small_font, "LV 2", 215, 572, white);
    draw_text(r, small_font, "HP", 295, 572, white);
    draw_hp_bar(r, 335, 570, player.hp, MAX_HP);
    snprintf(buffer, sizeof(buffer), "%d / %d", player.hp, MAX_HP);
    draw_text(r, small_font, buffer, 580, 572, white);

    for (i = 0; i < ACTION_COUNT; i++) {
        SDL_Rect button = { 45 + i * 305, 625, 270, 58 };
        SDL_Color color = (phase == PHASE_COMMAND && i == selected_action) ? white : orange;
        draw_outline(r, button, 4, color.r, color.g, color.b);
        draw_text(r, body_font, actions[i], button.x + 45, button.y + 12, color);
    }
}

const char *battle_result_message(void) {
    return won ? "캡스톤 제출 완료!" : "연결이 종료되었습니다.";
}

int battle_was_won(void) {
    return won;
}
