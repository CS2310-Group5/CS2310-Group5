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

// If `found` is given, found cells are drawn as a checkmark; `revealed` marks given-up answers.
void printGrid(const std::vector<std::string>& grid, int size,
               const std::vector<bool>* found = nullptr,
               const std::vector<bool>* revealed = nullptr) {
    std::cout << "    ";
    for (int c = 1; c <= size; ++c) std::cout << c << "    ";
    std::cout << '\n';
    for (int r = 0; r < size; ++r) {
        std::cout << static_cast<char>('A' + r) << "   ";
        for (int c = 0; c < size; ++c) {
            const int i = r * size + c;
            if (found && (*found)[i]) std::cout << "\xE2\x9C\x93" << "  ";   // ✓
            else if (revealed && (*revealed)[i]) std::cout << "\xE2\x9C\x97" << "  "; // ✗ (answer)
            else std::cout << grid[i] << "  ";
        }
        std::cout << '\n';
    }
}

void showTutorial() {
    std::cout << "\n=== HOW TO PLAY ===\n"
              << "Two emoji grids are shown: Grid A (original) and Grid B (changed).\n"
              << "Some cells in Grid B differ from Grid A. Find all of them!\n\n"
              << "Controls (type then press Enter):\n"
              << "  B2  - guess a cell (row letter + column number)\n"
              << "  H   - hint: tells you the row of one difference you haven't found\n"
              << "  G   - give up: reveals all remaining answers (you lose)\n"
              << "  R   - replay with a new puzzle, M - main menu, Q - quit\n\n"
              << "Rules:\n"
              << "  Found cells are marked with a checkmark in Grid B.\n"
              << "  3 wrong guesses and the round is over.\n\n"
              << "Scoring (on a win):\n"
              << "  1000 - elapsed seconds - (wrong guesses x 50), never below 0.\n";
    readLine("\nPress Enter to return to the main menu...");
}

std::string coord(int row, int col) {
    return std::string(1, static_cast<char>('A' + row)) + std::to_string(col + 1);
}

void printReveal(const Puzzle& p) {
    std::vector<bool> revealed(p.size * p.size, false);
    const auto remaining = getUnfoundAnswers(p);
    for (const auto& a : remaining) revealed[a.first * p.size + a.second] = true;
    std::cout << "\nRemaining answers (marked with \xE2\x9C\x97):\n";
    printGrid(p.modified, p.size, &p.found, &revealed);
    std::cout << "Answers:";
    for (const auto& a : remaining) std::cout << ' ' << coord(a.first, a.second);
    std::cout << '\n';
}

// Returns false to quit, true to return to the menu.
bool play(Settings settings) {
    const int kMaxErrors = 3;
    while (true) {  // one iteration per puzzle; replay starts a new one
        Puzzle p = generate(settings);
        int error_count = 0;
        std::string message;
        std::cout << "\n=== EMOJI SPOT THE DIFFERENCE ===\n";
        std::cout << "Theme: " << settings.theme.name << " | "
                  << settings.size << "x" << settings.size << " | Differences: "
                  << settings.differences << "\n";
        std::cout << "SHARE THIS CHALLENGE: theme=" << settings.theme.name
                  << ", size=" << settings.size << ", differences="
                  << settings.differences << ", code=" << settings.code << "\n";
        std::cout << "Note: Some terminals display emoji columns unevenly.\n";
        const auto start = std::chrono::steady_clock::now();
        bool replay = false;
        bool gameOver = false;
        while (true) {
            std::cout << "\nGrid A (original):\n";
            printGrid(p.original, p.size);
            std::cout << "\nGrid B (find changes here):\n";
            printGrid(p.modified, p.size, &p.found);
            std::cout << "\nFound " << p.foundCount << "/" << p.differences
                      << " | Wrong guesses " << error_count << "/" << kMaxErrors << "\n";
            if (!message.empty()) { std::cout << message << "\n"; message.clear(); }

            if (p.foundCount == p.differences) {
                const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - start).count();
                long long score = 1000 - static_cast<long long>(elapsed) - error_count * 50;
                if (score < 0) score = 0;
                std::cout << "YOU WIN! Time: " << elapsed << " seconds | Wrong guesses: "
                          << error_count << "\nFINAL SCORE: " << score << "\n";
                gameOver = true;
            } else if (error_count >= kMaxErrors) {
                std::cout << "GAME OVER - 3 wrong guesses!\n";
                printReveal(p);
                gameOver = true;
            }
            if (gameOver) {
                std::string action = readLine("R = replay new puzzle | M = menu | Q = quit: ");
                if (action == "q" || action == "Q") return false;
                if (action == "r" || action == "R") replay = true;
                break;
            }

            std::string input = readLine("Enter cell (e.g. B2), H=hint, G=give up, R=replay, M=menu, Q=quit: ");
            if (input == "q" || input == "Q") return false;
            if (input == "m" || input == "M") return true;
            if (input == "r" || input == "R") { replay = true; break; }
            if (input == "h" || input == "H") {
                if (auto hint = getRandomUnfoundHint(p))
                    message = std::string("HINT: there is a difference in row ")
                              + static_cast<char>('A' + hint->first) + ".";
                continue;
            }
            if (input == "g" || input == "G") {
                std::cout << "\nYou gave up.\n";
                printReveal(p);
                std::cout << "GAME OVER - you lost.\n";
                gameOver = true;
                std::string action = readLine("R = replay new puzzle | M = menu | Q = quit: ");
                if (action == "q" || action == "Q") return false;
                if (action == "r" || action == "R") replay = true;
                break;
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
                case GuessResult::Correct: message = "Correct!"; break;
                case GuessResult::Wrong: ++error_count; message = "Wrong! Try again."; break;
                case GuessResult::AlreadyFound: message = "Already found!"; break;
                case GuessResult::Invalid: message = "Outside the grid!"; break;
            }
        }
        if (replay) settings.code = newCode();
        else return true;
    }
}

int main() {
    std::vector<Theme> themes = loadThemes("themes/themes.txt");
    if (themes.empty()) themes = loadThemes("../themes/themes.txt");
    if (themes.empty()) {
        std::cout << "Using built-in themes (themes/themes.txt not found).\n";
        themes = defaultThemes();
    }
    std::mt19937 rng(std::random_device{}());
    std::cout << "WELCOME TO EMOJI SPOT THE DIFFERENCE!\n";
    while (true) {
        std::cout << "\n=== MAIN MENU ===\n1. New random puzzle\n2. Play puzzle code\n"
                     "3. Surprise me!\n4. How to play\nQ. Quit\n";
        std::string action = readLine("Choose: ");
        if (action == "Q" || action == "q") break;
        if (action == "4") { showTutorial(); continue; }
        if (action != "1" && action != "2" && action != "3") {
            std::cout << "Invalid option.\n"; continue;
        }
        Settings settings;
        if (action == "3") {
            std::uniform_int_distribution<std::size_t> pick(0, themes.size() - 1);
            settings.theme = themes[pick(rng)];
            std::cout << "\nSurprise! Your theme is: " << settings.theme.name << "\n";
        } else {
            std::cout << "\nChoose theme:\n";
            for (std::size_t i = 0; i < themes.size(); ++i)
                std::cout << (i + 1) << ". " << themes[i].name << '\n';
            int themeIndex = choiceInRange("Theme number (Q to quit): ", 1, static_cast<int>(themes.size()));
            if (themeIndex == -1) break;
            settings.theme = themes[themeIndex - 1];
        }
        std::cout << "\n1. Easy (4x4, 2 differences)\n"
                  << "2. Medium (5x5, 4 differences)\n"
                  << "3. Hard (6x6, 6 differences)\n"
                  << "4. Custom (choose grid size and number of differences)\n";
        int level = choiceInRange("Difficulty (Q to quit): ", 1, 4);
        if (level == -1) break;
        if (level == 4) {
            int n = choiceInRange("Grid size N (2-12, makes an NxN grid): ", 2, 12);
            if (n == -1) break;
            int m = choiceInRange("Number of differences M (1-" + std::to_string(n * n) + "): ", 1, n * n);
            if (m == -1) break;
            settings.size = n;
            settings.differences = m;
        } else {
            settings.size = level == 1 ? 4 : level == 2 ? 5 : 6;
            settings.differences = level == 1 ? 2 : level == 2 ? 4 : 6;
        }
        if (action != "2") {
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
