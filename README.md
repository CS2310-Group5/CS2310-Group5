# CS2310 Group 5 — Emoji Spot the Difference

A console-based, beginner-friendly Spot the Difference game and **reusable C++17 puzzle-generation library**. Players compare two emoji grids, enter coordinates to identify changed cells, and share puzzle codes to challenge friends.

## Course and Team Members

**Course:** CS2310  
**Group:** 5

Names are listed in **SURNAME Given Name** format.

| No. | Name |
| --- | --- |
| 1 | CHIU Wai Yan |
| 2 | CHOW Tsz Tung |
| 3 | LEE Hei Tung |
| 4 | WONG Cheuk Ying |
| 5 | WONG Lai Lam |

## Motivation
Traditional image-based Spot the Difference games require image assets and graphics libraries. We wanted a lightweight alternative that can be played in a terminal and adapted by other C++ developers for educational puzzles, memory games, and comparison challenges.

## Features
- Emoji themes: Animals, Food, Nature, Halloween; developers can add their own themes.
- Easy (4×4, 2 changes), Medium (5×5, 4 changes), Hard (6×6, 6 changes).
- Random generation with exactly the requested number of unique differences.
- Puzzle codes: use the **same code, theme, difficulty and version** to reproduce the same puzzle.
- Guess checking, duplicate-guess prevention, mistakes counter, completion time.
- Replay, menu and quit commands.
- A standalone `puzzle_generator` library and automated unit tests.

## Build and run (no third-party libraries)
Requirements: a C++17 compiler (g++ or clang++) and a UTF-8-capable terminal. CMake is optional.

From the repository root:

**macOS / Linux**
```sh
g++ -std=c++17 -Iinclude src/main.cpp src/PuzzleGenerator.cpp -o emoji_game
./emoji_game
```

**Windows (MinGW g++)**
```powershell
g++ -std=c++17 -Iinclude src/main.cpp src/PuzzleGenerator.cpp -o emoji_game.exe
.\emoji_game.exe
```

**CMake (any platform)**
```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
Run the resulting `emoji_game` executable from the repository root, so `themes/themes.txt` can be found (otherwise the built-in themes are used).

## How to play
1. Choose **1** for a new puzzle or **2** to enter a shared puzzle code.
2. Select an emoji theme and difficulty.
3. Compare Grid A with Grid B; enter a coordinate like `B2` where Grid B differs.
4. Enter `R` to replay with a **new** random puzzle, `M` for the main menu, or `Q` to quit.
5. Find every difference to win. The game shows your elapsed time and wrong guesses.

Example (the exact grids vary with code):
```text
=== MAIN MENU ===
1. New random puzzle
2. Play puzzle code
Q. Quit
Choose: 1
Theme number: 1
Difficulty: 1
SHARE THIS CHALLENGE: theme=Animals, size=4, differences=2, code=...
Enter cell (e.g. B2), R=replay, M=menu, Q=quit: B2
Correct!
```

## Developer customization: add your own emoji theme
Edit `themes/themes.txt` (UTF-8) and add one line:
```text
Space|🚀,🪐,🌎,🌕,⭐,☄️
```
Format: `ThemeName|symbol1,symbol2,symbol3,...` (at least two distinct symbols). Run the game again and choose the new theme. **No C++ changes or recompilation are required.**

If the file is missing or has no valid themes, the game uses built-in themes. Malformed theme lines and duplicate-symbol themes are ignored.

## Developer API: use the engine in another program
`include/PuzzleGenerator.h` declares the public API, implemented in `src/PuzzleGenerator.cpp`. Example:
```cpp
#include "PuzzleGenerator.h"
#include <iostream>
int main() {
    emoji_puzzle::Settings s;
    s.size = 4;
    s.differences = 2;
    s.theme = emoji_puzzle::defaultThemes()[0];
    s.code = 12345;
    auto p = emoji_puzzle::generate(s);
    auto result = emoji_puzzle::guess(p, 1, 1); // B2
    std::cout << "Found: " << p.foundCount << "\n";
}
```
Compile this example with `-Iinclude` and `src/PuzzleGenerator.cpp`, or link the `puzzle_generator` CMake target. The API separates puzzle generation/validation from console display and menus.

## Innovation and limitations
Our distinctive combination is **data-driven emoji themes, exact-difference puzzle generation, reusable generation/guessing API, and replayable challenge codes**, all without graphics dependencies. This makes it useful for fast terminal games and educational demos. These ideas exist individually elsewhere; we do **not** claim to have invented procedural generation or seeded random numbers. The innovation argument is the accessible, customizable combination for this use case.

Puzzle codes are stable within this project version because the engine uses an explicitly implemented fixed xorshift32 generator. If the algorithm or theme ordering changes, old codes may yield different puzzles. A puzzle challenge is identified by **theme + difficulty + code + project version**. The random generator is for games, **not cryptography**. Emoji width varies by terminal, so some grid columns may not align perfectly.

## Testing
```sh
g++ -std=c++17 -Iinclude tests/test_generator.cpp src/PuzzleGenerator.cpp -o generator_tests
./generator_tests
```
Tests check reproducibility, exact difference counts for all difficulties, guess handling, and invalid settings. Or use `ctest` after CMake build.

## GitHub collaboration
- This project lives in the existing public `CS2310-Group5/CS2310-Group5` repository.
- Give each member a feature branch and have them make **genuine commits** for changes they personally implement or verify.
- Agree on five concrete code contributions before splitting work; record actual names and contributions below.
- Use pull requests to review changes before merging to `main`.


