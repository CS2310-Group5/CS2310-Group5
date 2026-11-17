#include "PuzzleGenerator.h"
#include <chrono>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace emoji_puzzle;

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) return "Q";
    return line;
}

int choiceInRange(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        std::string s = readLine(prompt);
        if (s == "q" || s == "Q") return -1;
        std::istringstream in(s);
        int n; char extra;
        if ((in >> n) && !(in >> extra) && n >= minimum && n <= maximum) return n;
        std::cout << "Invalid choice. Try again, or Q to quit.\n";
    }
}

std::uint32_t newCode() {
    static std::mt19937 generator(std::random_device{}());
    return generator();
}

bool parseCode(const std::string& s, std::uint32_t& code) {
    if (s.empty()) return false;
    for (char c : s) if (c < '0' || c > '9') return false;
    try {
        unsigned long long n = std::stoull(s);
        if (n > std::numeric_limits<std::uint32_t>::max()) return false;
        code = static_cast<std::uint32_t>(n);
        return true;
    } catch (...) { return false; }
}

void printGrid(const std::vector<std::string>& grid, int size) {
    std::cout << "    ";
    for (int c = 1; c <= size; ++c) std::cout << c << "    ";
    std::cout << '\n';
    for (int r = 0; r < size; ++r) {
        std::cout << static_cast<char>('A' + r) << "   ";
        for (int c = 0; c < size; ++c) {
            std::cout << grid[r * size + c] << "  ";
        }
        std::cout << '\n';
    }
}

// Returns false to quit, true to return to the menu.
bool play(Settings settings) {
new_puzzle:
    Puzzle p = generate(settings);
    std::cout << "\n=== EMOJI SPOT THE DIFFERENCE ===\n";
    std::cout << "Theme: " << settings.theme.name << " | "
              << settings.size << "x" << settings.size << " | Differences: "
              << settings.differences << "\n";
    std::cout << "SHARE THIS CHALLENGE: theme=" << settings.theme.name
              << ", size=" << settings.size << ", differences="
              << settings.differences << ", code=" << settings.code << "\n";
    std::cout << "Note: Some terminals display emoji columns unevenly.\n";
    auto start = std::chrono::steady_clock::now();
    while (true) {
        std::cout << "\nGrid A (original):\n";
        printGrid(p.original, p.size);
        std::cout << "\nGrid B (find changes here):\n";
        printGrid(p.modified, p.size);
        std::cout << "\nFound " << p.foundCount << "/" << p.differences
                  << " | Wrong guesses " << p.mistakes << "\n";
        if (p.foundCount == p.differences) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start).count();
            std::cout << "YOU WIN! Time: " << elapsed << " seconds.\n";
            std::string action = readLine("R = replay new puzzle | M = menu | Q = quit: ");
            if (action == "q" || action == "Q") return false;
            if (action == "r" || action == "R") {
                settings.code = newCode();
                goto new_puzzle;
            }
            return true;
        }
        std::string input = readLine("Enter cell (e.g. B2), R=replay, M=menu, Q=quit: ");
        if (input == "q" || input == "Q") return false;
        if (input == "m" || input == "M") return true;
        if (input == "r" || input == "R") {
            settings.code = newCode();
            goto new_puzzle;
        }
        if (input.size() < 2 || input.size() > 3) {
            std::cout << "Invalid coordinate. Example: B2\n";
            continue;
        }
        const int row = std::toupper(static_cast<unsigned char>(input[0])) - 'A';
        int col = 0;
        bool valid = true;
        for (std::size_t i = 1; i < input.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(input[i]))) { valid = false; break; }
            col = col * 10 + (input[i] - '0');
        }
        if (!valid) { std::cout << "Invalid coordinate.\n"; continue; }
        switch (guess(p, row, col - 1)) {
            case GuessResult::Correct: std::cout << "Correct!\n"; break;
            case GuessResult::Wrong: std::cout << "Wrong! Try again.\n"; break;
            case GuessResult::AlreadyFound: std::cout << "Already found!\n"; break;
            case GuessResult::Invalid: std::cout << "Outside the grid!\n"; break;
        }
    }
}

int main() {
    std::vector<Theme> themes = loadThemes("themes/themes.txt");
    if (themes.empty()) themes = loadThemes("../themes/themes.txt");
    if (themes.empty()) {
        std::cout << "Using built-in themes (themes/themes.txt not found).\n";
        themes = defaultThemes();
    }
    std::cout << "WELCOME TO EMOJI SPOT THE DIFFERENCE!\n";
    while (true) {
        std::cout << "\n=== MAIN MENU ===\n1. New random puzzle\n2. Play puzzle code\nQ. Quit\n";
        std::string action = readLine("Choose: ");
        if (action == "Q" || action == "q") break;
        if (action != "1" && action != "2") {
            std::cout << "Invalid option.\n"; continue;
        }
        std::cout << "\nChoose theme:\n";
        for (std::size_t i = 0; i < themes.size(); ++i)
            std::cout << (i + 1) << ". " << themes[i].name << '\n';
        int themeIndex = choiceInRange("Theme number (Q to quit): ", 1, static_cast<int>(themes.size()));
        if (themeIndex == -1) break;
        std::cout << "\n1. Easy (4x4, 2 differences)\n"
                  << "2. Medium (5x5, 4 differences)\n"
                  << "3. Hard (6x6, 6 differences)\n";
        int level = choiceInRange("Difficulty (Q to quit): ", 1, 3);
        if (level == -1) break;
        Settings settings;
        settings.theme = themes[themeIndex - 1];
        settings.size = level == 1 ? 4 : level == 2 ? 5 : 6;
        settings.differences = level == 1 ? 2 : level == 2 ? 4 : 6;
        if (action == "1") {
            settings.code = newCode();
        } else {
            while (true) {
                std::string input = readLine("Enter numeric puzzle code (Q to quit): ");
                if (input == "Q" || input == "q") return 0;
                if (parseCode(input, settings.code)) break;
                std::cout << "Code must be an integer from 0 to 4294967295.\n";
            }
        }
        if (!play(settings)) break;
    }
    std::cout << "Thanks for playing!\n";
}
