# VS Code 개발 환경 설정 정리

> 「정통에서 살아남기」 프로젝트의 VS Code 설정 + 익스텐션 현황.
> 새 팀원이 동일 환경을 맞출 때 참고. (기준일: 2026-06-06)

---

## VS Code 버전

| 항목 | 값 |
|------|-----|
| Version | 1.123.0 |
| Commit | 6a44c352bd24569c417e530095901b649960f9f8 |
| Arch | x64 |

---

## 설치된 익스텐션

| 익스텐션 | ID | 버전 | 용도 |
|----------|-----|------|------|
| C/C++ | `ms-vscode.cpptools` | 1.32.2 | C 언어 IntelliSense, 디버깅 (cppvsdbg) |
| Python | `ms-python.python` | 2026.4.0 | Python 언어 지원 |
| Pylance | `ms-python.vscode-pylance` | 2026.2.1 | Python 타입 체크 / IntelliSense |
| Python Debugger | `ms-python.debugpy` | 2026.6.0 | Python 디버깅 |
| Python Environments | `ms-python.vscode-python-envs` | 1.30.0 | Python 가상환경 관리 |
| Korean Language Pack | `ms-ceintl.vscode-language-pack-ko` | 1.123.2026060414 | VS Code UI 한글화 |
| Claude Code | `anthropic.claude-code` | 2.1.165 | AI 코딩 보조 |

> 이 프로젝트는 **C 언어 기반**이라 실제로 필요한 건 `ms-vscode.cpptools` 와 `anthropic.claude-code`.
> Python 계열 4종은 다른 작업용으로 보이며 본 프로젝트 빌드에는 무관.

### 재설치 (한 번에)

```powershell
code --install-extension ms-vscode.cpptools
code --install-extension anthropic.claude-code
code --install-extension ms-ceintl.vscode-language-pack-ko
# (Python 작업도 한다면)
code --install-extension ms-python.python
code --install-extension ms-python.vscode-pylance
code --install-extension ms-python.debugpy
code --install-extension ms-python.vscode-python-envs
```

---

## 사용자(User) 설정

위치: `%APPDATA%\Code\User\settings.json`

```json
{
    "claudeCode.preferredLocation": "panel",
    "workbench.colorTheme": "Dark Modern"
}
```

| 키 | 값 | 의미 |
|----|-----|------|
| `claudeCode.preferredLocation` | `panel` | Claude Code를 하단 패널에 표시 |
| `workbench.colorTheme` | `Dark Modern` | 다크 테마 |

> 사용자 keybindings.json 은 없음 (기본 단축키 사용).

---

## 워크스페이스(`.vscode/`) 설정

워크스페이스 전용 `settings.json` 은 **없음**. 아래 3개 파일만 존재.

### 1. `c_cpp_properties.json` — IntelliSense 구성

| 항목 | 값 |
|------|-----|
| name | Win32 |
| includePath | `src`, `SDL2-2.30.12/include`, `SDL2_ttf-2.22.0/include` |
| defines | `_DEBUG`, `UNICODE`, `_UNICODE` |
| compilerPath | `cl.exe` (MSVC) |
| cStandard | c17 |
| cppStandard | c++17 |
| intelliSenseMode | windows-msvc-x64 |

### 2. `tasks.json` — 빌드 태스크 (MSBuild)

빌드 도구: `MSBuild.exe` (VS 2022 Community)
대상 프로젝트: `Survice ICE\Survice ICE.vcxproj`

| 태스크 label | 동작 | 비고 |
|--------------|------|------|
| `MSBuild: Debug x64` | Debug 빌드 | **기본 빌드 태스크** (`Ctrl+Shift+B`) |
| `MSBuild: Release x64` | Release 빌드 | |
| `MSBuild: Rebuild Debug x64` | Debug 전체 재빌드 | |
| `MSBuild: Clean` | 빌드 산출물 정리 | |

### 3. `launch.json` — 디버그/실행 구성

산출물: `Survice ICE\x64\Debug\Survice ICE.exe`
디버거 타입: `cppvsdbg`, 사전 태스크: `MSBuild: Debug x64`

| 구성 name | 동작 |
|-----------|------|
| `Debug (cppvsdbg, x64)` | 디버깅 실행 (F5) |
| `Run (no debug)` | 디버깅 없이 실행 (`noDebug: true`) |

---

## 빌드 / 실행 흐름 요약

1. `Ctrl+Shift+B` → `MSBuild: Debug x64` 태스크로 빌드
2. `F5` → 빌드 후 `Survice ICE.exe` 디버깅 실행
3. 실제 컴파일·링크 설정(SDL2 링크, `winmm.lib` 등)은 VS 프로젝트(`Survice ICE.vcxproj`)에 있음 — `.vcxproj` 가 단일 진실 공급원(SSOT)