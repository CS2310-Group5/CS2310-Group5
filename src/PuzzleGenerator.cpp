#include "PuzzleGenerator.h"
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace emoji_puzzle {
namespace {
// Fixed algorithm: identical puzzle codes reproduce the same puzzle across platforms.
class Random {
    std::uint32_t state;
public:
    explicit Random(std::uint32_t seed) : state(seed == 0 ? 0x6D2B79F5u : seed) {}
    std::uint32_t next() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }
    std::uint32_t below(std::uint32_t limit) { return next() % limit; }
};

std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}
}

std::vector<Theme> defaultThemes() {
    return {
        {"Animals", {"🐱", "🐶", "🐼", "🐸", "🐰", "🦊", "🐻", "🐯"}},
        {"Food", {"🍎", "🍌", "🍇", "🍓", "🍕", "🍔", "🍩", "🍪"}},
        {"Nature", {"🌸", "🌻", "🌳", "🍄", "🌵", "🍀", "🌷", "🌹"}}
    };
}

std::vector<Theme> loadThemes(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return {};
    std::vector<Theme> result;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        const auto bar = line.find('|');
        if (bar == std::string::npos) continue;
        Theme theme;
        theme.name = trim(line.substr(0, bar));
        std::stringstream ss(line.substr(bar + 1));
        std::string symbol;
        while (std::getline(ss, symbol, ',')) {
            symbol = trim(symbol);
            if (!symbol.empty()) theme.symbols.push_back(symbol);
        }
        if (!theme.name.empty() && theme.symbols.size() >= 2) {
            bool unique = true;
            for (std::size_t i = 0; i < theme.symbols.size(); ++i)
                for (std::size_t j = i + 1; j < theme.symbols.size(); ++j)
                    if (theme.symbols[i] == theme.symbols[j]) unique = false;
            if (unique) result.push_back(std::move(theme));
        }
    }
    return result;
}

Puzzle generate(const Settings& settings) {
    if (settings.size < 2 || settings.size > 12)
        throw std::invalid_argument("Grid size must be between 2 and 12");
    const int cells = settings.size * settings.size;
    if (settings.differences < 1 || settings.differences > cells)
        throw std::invalid_argument("Invalid number of differences");
    if (settings.theme.symbols.size() < 2)
        throw std::invalid_argument("Theme needs at least two different symbols");
    for (std::size_t i = 0; i < settings.theme.symbols.size(); ++i)
        for (std::size_t j = i + 1; j < settings.theme.symbols.size(); ++j)
            if (settings.theme.symbols[i] == settings.theme.symbols[j])
                throw std::invalid_argument("Theme symbols must be unique");

    Random rng(settings.code);
    Puzzle p;
    p.size = settings.size;
    p.differences = settings.differences;
    p.original.resize(cells);
    p.found.assign(cells, false);
    const auto count = static_cast<std::uint32_t>(settings.theme.symbols.size());
    for (auto& cell : p.original) cell = settings.theme.symbols[rng.below(count)];
    p.modified = p.original;

    std::vector<int> positions(cells);
    for (int i = 0; i < cells; ++i) positions[i] = i;
    // Partial Fisher-Yates shuffle: no duplicate changed positions.
    for (int i = 0; i < settings.differences; ++i) {
        const int j = i + static_cast<int>(rng.below(static_cast<std::uint32_t>(cells - i)));
        std::swap(positions[i], positions[j]);
        const int pos = positions[i];
        // Choose one of the other symbols, never the original.
        std::uint32_t oldIndex = 0;
        while (settings.theme.symbols[oldIndex] != p.original[pos]) ++oldIndex;
        std::uint32_t newIndex = rng.below(count - 1);
        if (newIndex >= oldIndex) ++newIndex;
        p.modified[pos] = settings.theme.symbols[newIndex];
    }
    return p;
}

std::vector<std::pair<int, int>> getUnfoundAnswers(const Puzzle& p) {
    std::vector<std::pair<int, int>> result;
    for (int i = 0; i < p.size * p.size; ++i)
        if (p.original[i] != p.modified[i] && !p.found[i])
            result.emplace_back(i / p.size, i % p.size);
    return result;
}

std::optional<std::pair<int, int>> getRandomUnfoundHint(const Puzzle& p) {
    const auto remaining = getUnfoundAnswers(p);
    if (remaining.empty()) return std::nullopt;
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, remaining.size() - 1);
    return remaining[dist(gen)];
}

GuessResult guess(Puzzle& p, int row, int col) {
    if (row < 0 || col < 0 || row >= p.size || col >= p.size) return GuessResult::Invalid;
    const int index = row * p.size + col;
    if (p.found[index]) return GuessResult::AlreadyFound;
    if (p.original[index] != p.modified[index]) {
        p.found[index] = true;
        ++p.foundCount;
        return GuessResult::Correct;
    }
    ++p.mistakes;
    return GuessResult::Wrong;
}
}
