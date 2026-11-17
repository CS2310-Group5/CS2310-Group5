#ifndef PUZZLE_GENERATOR_H
#define PUZZLE_GENERATOR_H

#include <cstdint>
#include <string>
#include <vector>

namespace emoji_puzzle {
struct Theme {
    std::string name;
    std::vector<std::string> symbols;
};

struct Settings {
    int size = 4;
    int differences = 2;
    Theme theme;
    std::uint32_t code = 1;
};

struct Puzzle {
    int size = 0;
    std::vector<std::string> original;
    std::vector<std::string> modified;
    std::vector<bool> found;
    int differences = 0;
    int foundCount = 0;
    int mistakes = 0;
};

enum class GuessResult { Correct, Wrong, AlreadyFound, Invalid };

Puzzle generate(const Settings& settings);
GuessResult guess(Puzzle& puzzle, int row, int col);
std::vector<Theme> loadThemes(const std::string& filename);
std::vector<Theme> defaultThemes();
}
#endif
