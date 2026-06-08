# VS → VS Code 전환 트러블슈팅 기록

Visual Studio 2022로 만든 `Survice ICE.vcxproj` 프로젝트를 VS Code에서 열어 개발하면서 겪은 문제와 해결 방법 정리.

---

## 1. C/C++ 확장(extension)이 안 깔려 있음

### 증상
- VS Code에서 `.c` / `.h` 파일을 열어도 문법 강조 색깔 정도만 들어오고 자동완성·정의로 이동·오류 표시(빨간 줄)가 전혀 안 됐다.
- F12(정의로 이동), Ctrl+클릭(헤더 따라가기) 작동 안 함.

### 원인
- VS는 C/C++ 지원이 내장이지만, VS Code는 빈 에디터에 가까워서 언어별 확장을 직접 설치해야 한다.

### 해결
- VS Code 좌측 사이드바 **Extensions(Ctrl+Shift+X)** 에서 **C/C++** (Microsoft, 식별자: `ms-vscode.cpptools`) 설치.
- 설치 후 IntelliSense·디버거가 활성화됨.

---

## 2. SDL2 헤더를 찾지 못함 (IntelliSense 빨간 줄)

### 증상
- 모든 `.c` 파일에서 `#include <SDL.h>`, `#include <SDL_ttf.h>` 줄에 빨간 줄.
- `cannot open source file "SDL.h"` 같은 IntelliSense 오류.
- 신기하게 **VS에서는 빌드가 잘 됐다** — 즉 컴파일러는 헤더를 찾지만 VS Code의 IntelliSense만 못 찾는 상태.

### 원인
- VS는 솔루션의 *프로젝트 속성 → C/C++ → 일반 → 추가 포함 디렉터리* 에 SDL2 경로가 등록돼 있어서 컴파일 시 자동으로 사용.
- VS Code의 IntelliSense는 그 설정을 모름. `.vcxproj`를 직접 파싱하지 않고 `c_cpp_properties.json`만 본다.

### 해결
`.vscode/c_cpp_properties.json` 파일을 만들고 SDL2 헤더 경로를 `includePath`에 추가.

```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/src",
                "${workspaceFolder}/SDL2-2.30.12/include",
                "${workspaceFolder}/SDL2_ttf-2.22.0/include"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "windowsSdkVersion": "10.0",
            "compilerPath": "cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-msvc-x64"
        }
    ],
    "version": 4
}
```

핵심 항목:
| 키 | 역할 |
|----|------|
| `includePath` | IntelliSense가 헤더 검색할 폴더 — 여기 SDL2 / SDL2_ttf include 경로 추가가 핵심 |
| `defines` | VS 프로젝트에 정의돼 있던 `_DEBUG`, `UNICODE`, `_UNICODE` 동일 적용 |
| `compilerPath` | `cl.exe` → MSVC 컴파일러 (VS 설치 시 같이 깔림) |
| `intelliSenseMode` | `windows-msvc-x64` → MSVC 기준으로 분석 (GCC/Clang 아님) |

저장하면 즉시 IntelliSense가 재분석 → 빨간 줄 사라짐.

---

## 3. VS Code에서 빌드 못 함

### 증상
- VS에서는 **F5** 또는 **Ctrl+Shift+B** 로 빌드/실행이 자동.
- VS Code에서 **Ctrl+Shift+B** 누르면 "구성된 빌드 작업이 없습니다" 메시지.

### 원인
- VS Code는 빌드 명령을 모름. 우리 프로젝트는 MSBuild 기반(`.vcxproj`)인데 VS Code엔 그 정보가 없다.

### 해결 (두 가지 길)
1. **MSBuild 그대로 사용 (선택한 방법)** — VS의 `.vcxproj`를 유지하고 VS Code에서는 MSBuild만 호출.
2. CMake 등으로 빌드 시스템 통째 교체 — 본 프로젝트엔 과함.

`.vscode/tasks.json` 작성:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "MSBuild: Debug x64",
            "type": "shell",
            "command": "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
            "args": [
                "${workspaceFolder}\\Survice ICE\\Survice ICE.vcxproj",
                "/t:Build",
                "/p:Configuration=Debug",
                "/p:Platform=x64",
                "/nologo",
                "/v:minimal"
            ],
            "group": { "kind": "build", "isDefault": true },
            "problemMatcher": "$msCompile"
        }
    ]
}
```

핵심:
- `command` — VS 2022 Community에 포함된 MSBuild.exe 절대 경로.
- `args` — `.vcxproj` 경로 + `/t:Build` (빌드) + `/p:Configuration=Debug` + `/p:Platform=x64`.
- `problemMatcher: "$msCompile"` — MSVC 에러 형식을 VS Code가 인식해서 "문제(Problems)" 탭에 표시.
- `isDefault: true` — Ctrl+Shift+B 누르면 이 작업이 기본 실행.

같은 패턴으로 **Release x64**, **Rebuild Debug x64**, **Clean** 작업도 추가.

---

## 4. 디버깅 안 됨 (브레이크포인트 안 잡힘)

### 증상
- F5 눌러도 디버거가 안 붙거나, 실행은 되는데 브레이크포인트 무시됨.

### 원인
- VS Code 기본 디버거(`cppdbg` = GDB/LLDB 용)는 MSVC가 만든 `.pdb` 디버그 정보를 못 읽는다.
- MSVC로 빌드한 결과물은 VS 디버거(cppvsdbg)로 디버깅해야 함.

### 해결
`.vscode/launch.json` 작성 — `type: "cppvsdbg"` 사용.

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug (cppvsdbg, x64)",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}\\Survice ICE\\x64\\Debug\\Survice ICE.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal",
            "preLaunchTask": "MSBuild: Debug x64"
        }
    ]
}
```

핵심:
| 키 | 역할 |
|----|------|
| `type: "cppvsdbg"` | VS의 디버거 엔진 사용 — MSVC의 `.pdb` 인식 |
| `program` | MSBuild가 만든 출력 exe 경로 (`Survice ICE/x64/Debug/Survice ICE.exe`) |
| `cwd` | 작업 디렉터리 — `${workspaceFolder}`로 두면 상대경로(`assets/...`) 로드가 VS 실행 환경과 동일 |
| `preLaunchTask` | F5 누르면 빌드부터 자동 실행 |
| `console: "integratedTerminal"` | VS Code 내장 터미널에 게임 콘솔 출력 표시 |

---

## 5. 새 `.c` / `.h` 추가 시 빌드에 안 잡힘 (주의 사항)

### 증상
- `src/` 또는 `src/phase3/` 하위에 새 파일을 만들었는데, VS Code에선 IntelliSense는 잘 잡히지만 빌드(F5/Ctrl+Shift+B) 시 **링크 에러** 또는 함수가 정의되지 않았다는 오류.

### 원인
- `c_cpp_properties.json`의 `includePath`는 IntelliSense 전용이라 헤더 검색만 영향.
- **실제 빌드**는 MSBuild가 `.vcxproj`에 명시된 파일 목록을 컴파일함. 새 파일은 `.vcxproj` 안에 없으니 빌드 안 됨.

### 해결
- `Survice ICE/Survice ICE.vcxproj` 에 `<ClCompile Include="..\src\새파일.c" />` 추가.
- `Survice ICE/Survice ICE.vcxproj.filters` 에 같은 파일을 적절한 필터 아래에 추가 (Solution Explorer에서 카테고리별로 묶기 위함).
- VS와 VS Code 양쪽 다 빌드에 잡힘.


---

## 정리: VS Code가 일을 하는 흐름

```
[ 편집 ]
   VS Code
      │
      ▼
[ IntelliSense (코드 분석) ]
   c_cpp_properties.json
      → includePath / defines / compilerPath
      → SDL2 헤더, MSVC, _DEBUG 등 인식
      │
      ▼
[ 빌드 (Ctrl+Shift+B / F5) ]
   tasks.json → MSBuild.exe
      → .vcxproj 읽어 컴파일·링크
      → Survice ICE/x64/Debug/Survice ICE.exe 생성
      │
      ▼
[ 디버깅 (F5) ]
   launch.json → cppvsdbg
      → 위 exe 실행 + .pdb로 브레이크포인트
```

VS와 차이점은 **하나의 시스템(VS) → 세 개의 JSON 파일(VS Code)** 로 역할이 분리됐다는 점뿐.
빌드 자체는 동일하게 MSBuild가 담당하므로 결과 바이너리는 두 환경에서 똑같다.
