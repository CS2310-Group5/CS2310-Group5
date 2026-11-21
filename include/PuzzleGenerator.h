#ifndef PUZZLE_GENERATOR_H
#define PUZZLE_GENERATOR_H

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
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

// Settings::size is N (grid is N x N, 2..12) and Settings::differences is M
// (unique changed cells, 1..N*N). Throws std::invalid_argument if M > N*N.
Puzzle generate(const Settings& settings);

// (row, col) of every difference the player has not found yet.
std::vector<std::pair<int, int>> getUnfoundAnswers(const Puzzle& puzzle);
// (row, col) of one random unfound difference, or std::nullopt if all are found.
std::optional<std::pair<int, int>> getRandomUnfoundHint(const Puzzle& puzzle);
GuessResult guess(Puzzle& puzzle, int row, int col);
std::vector<Theme> loadThemes(const std::string& filename);
std::vector<Theme> defaultThemes();
}
#endif
