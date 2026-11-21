#include "PuzzleGenerator.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace emoji_puzzle;
int main() {
    Settings s;
    s.theme = defaultThemes()[0];
    s.code = 12345;
    // Each case is {N, M}; includes custom values.
    const int cases[][2] = {{4, 2}, {5, 4}, {6, 6}, {3, 9}, {7, 1}};
    for (const auto& cs : cases) {
        const int size = cs[0];
        s.size = size;          // N
        s.differences = cs[1];  // M
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
        assert(getUnfoundAnswers(a).empty());
        assert(!getRandomUnfoundHint(a).has_value());
        Puzzle c = generate(s);
        auto all = getUnfoundAnswers(c);
        assert(static_cast<int>(all.size()) == s.differences);
        auto hint = getRandomUnfoundHint(c);
        assert(hint.has_value());
        assert(guess(c, hint->first, hint->second) == GuessResult::Correct);
        assert(static_cast<int>(getUnfoundAnswers(c).size()) == s.differences - 1);
    }
    bool thrown = false;
    s.size = 1;
    try { generate(s); } catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
    // More emojis (M) than grid cells (N*N) must be rejected.
    s.size = 3; s.differences = 10;
    thrown = false;
    try { generate(s); } catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
    s.differences = 0;
    thrown = false;
    try { generate(s); } catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
    std::cout << "All generator tests passed.\n";
}
