#include "PuzzleGenerator.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace emoji_puzzle;
int main() {
    Settings s;
    s.theme = defaultThemes()[0];
    s.code = 12345;
    for (int size : {4, 5, 6}) {
        s.size = size;
        s.differences = size == 4 ? 2 : size == 5 ? 4 : 6;
        Puzzle a = generate(s);
        Puzzle b = generate(s);
        assert(a.original == b.original && a.modified == b.modified);
        int changed = 0;
        for (int i = 0; i < size * size; ++i)
            if (a.original[i] != a.modified[i]) ++changed;
        assert(changed == s.differences);
        assert(guess(a, -1, 0) == GuessResult::Invalid);
        for (int i = 0; i < size * size; ++i) {
            int row = i / size, col = i % size;
            if (a.original[i] != a.modified[i]) {
                assert(guess(a, row, col) == GuessResult::Correct);
                assert(guess(a, row, col) == GuessResult::AlreadyFound);
            } else {
                assert(guess(a, row, col) == GuessResult::Wrong);
            }
        }
        assert(a.foundCount == s.differences);
    }
    bool thrown = false;
    s.size = 1;
    try { generate(s); } catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
    std::cout << "All generator tests passed.\n";
}
