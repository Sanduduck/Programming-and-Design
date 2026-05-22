# 수정 내역 정리

`src/` 폴더의 `.c`/`.h` 파일에서 오류 검출 코드와 더미 데이터를 모두 제거했다.

---

## 1. `src/main.c`

SDL/TTF 초기화 실패 검사 코드를 모두 제거.

- `SDL_Init` 실패 시 `printf` 출력 + `return 1` 처리 제거
- `TTF_Init` 실패 시 `SDL_Quit()` + `return 1` 처리 제거
- `SDL_CreateWindow` 실패 시 `printf` + 정리 후 `return 1` 처리 제거
- `SDL_CreateRenderer` 실패 시 `printf` + 정리 후 `return 1` 처리 제거

→ 초기화 함수만 일렬로 호출하는 형태로 단순화.

---

## 2. `src/ui_menu.c`

폰트 관련 NULL 체크와 실패 메시지 제거.

- `load_font()` 의 `if (!menu_font) printf("폰트 로드 실패: ...")` 제거
- `draw_text_centered()` 의 `if (!menu_font) return;` 제거
- `draw_text_centered()` 의 `if (!surf) return;` (TTF Surface NULL 체크) 제거
- 더 이상 `printf` 가 없으므로 `#include <stdio.h>` 제거

---

## 3. `src/ui_ranking.c`

더미 랭킹 데이터와 NULL 체크 제거.

- 랭킹 배열 더미 초기값 제거
  ```c
  // 제거 전
  static RankEntry ranking[RANK_COUNT] = {
      { -1, "" }, { -1, "" }, ... { -1, "" }
  };

  // 제거 후
  static RankEntry ranking[RANK_COUNT];
  ```
- `load_fonts()` 의 `if (!title_font || !item_font) printf(...)` 제거
- `draw_text_centered()` 의 `if (!font) return;` 제거
- `draw_text_centered()` 의 `if (!surf) return;` 제거

---

## 4. `src/ui_result.c`

표시되지도 않는 더미 학점 계산 호출 제거.

- 다음 더미 코드 제거
  ```c
  char grade[4];
  calc_grade(total_score, player.hp, grade);
  (void)grade;
  ```
- 더 이상 사용하지 않는 `#include "player.h"`, `#include "score.h"` 제거

---

## 5. `src/collision.c`

플레이어 크기 더미값 제거.

- `// 임시 플레이어 크기 (40 x 60)` 주석 제거
- `int pw = 40, ph = 60;` → `int pw = 50, ph = 50;`
  (`player.c` 의 `PLAYER_WIDTH/HEIGHT` 값과 일치시킴)

---

## 변경되지 않은 파일

- `types.h`
- `game_state.h`, `game_state.c`
- `player.h`, `player.c`
- `obstacle.h`, `obstacle.c`
- `pattern.h`, `pattern.c`
- `collision.h`
- `score.h`, `score.c`
- `ui_menu.h`, `ui_play.h`, `ui_play.c`
- `ui_result.h`
- `ui_ranking.h`
- `ui_settings.h`, `ui_settings.c`

---

## 참고

- `obstacle.c`, `pattern.c` 의 `(void)dt;` 같은 미사용 인자 처리는 컴파일 경고 회피용으로 둠
  (오류 검출도 더미 데이터도 아님).
- `player.c` 의 `if (player.invincible_timer > 0) return;` 은 무적 시간 게임 로직이므로 그대로 둠.
