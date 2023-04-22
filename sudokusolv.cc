// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <random>

class SudokuBoard
{
public:
    SudokuBoard()
    {
        _init();
    }

    SudokuBoard(const std::initializer_list<std::initializer_list<uint8_t>> &initPos)
    {
        _init();

        auto iIt = initPos.begin();
        for (int i=0; i < 9; ++i, ++iIt) {
            auto jIt = iIt->begin();
            for (int j=0; j < 9; ++j, ++jIt) {
                _set(*jIt, i, j);
            }
        }
    }

    // returns true iff there is a field which cannot be set directly
    bool isAnyDirectlyImpossible() const
    {
        for (int rowIdx = 0; rowIdx < 9; ++rowIdx) {
            for (int colIdx = 0; colIdx < 9; ++colIdx) {
                if (_board[rowIdx][colIdx] > 0 && _board[rowIdx][colIdx] < 10)
                    // there already is a fixed number attached to the field
                    continue;

                // check if it is possible to attach some number to the current field
                int k;
                for (k = 1; k < 10; ++k) {
                    int mask = 1 << (k - 1);

                    // check if the number can be set horizontally, vertically as well as
                    // in the 3x3 block
                    int blockIdx = (rowIdx/3) + (colIdx/3)*3;
                    if (!(_verticalSet[colIdx] & mask) &&
                        !(_horizontalSet[rowIdx] & mask) &&
                        !(_blockSet[blockIdx] & mask))
                        // number can be placed in the current field
                        break;
                }

                // no number could be placed on the current field
                if (k == 10)
                    return true;
            }
        }

        // a number could be placed on every free field
        return false;
    }

    bool set(uint8_t num, uint8_t rowIdx, uint8_t colIdx)
    {
        if (num > 0 && num < 10) {
            int mask = 1 << (num - 1);

            // horizontal line
            if (_verticalSet[colIdx] & mask)
                return false;
            // vertical line
            else if (_horizontalSet[rowIdx] & mask)
                return false;

            // 3x3 block
            int blockIdx = (rowIdx/3) + (colIdx/3)*3;
            if (_blockSet[blockIdx] & mask)
                return false;

            _verticalSet[colIdx] |= mask;
            _horizontalSet[rowIdx] |= mask;
            _blockSet[blockIdx] |= mask;
        }
        _board[rowIdx][colIdx] = num;

        return true;
    }

    void unset(uint8_t new_val, uint8_t rowIdx, uint8_t colIdx)
    {
        uint8_t cur_val = _board[rowIdx][colIdx];

        if (cur_val > 0 && cur_val < 10) {
            int mask = ~(1 << (cur_val - 1));

            int blockIdx = (rowIdx/3) + (colIdx/3)*3;

            _verticalSet[colIdx] &= mask;
            _horizontalSet[rowIdx] &= mask;
            _blockSet[blockIdx] &= mask;
        }

        _board[rowIdx][colIdx] = new_val;
    }

    void print(bool copyAndPastableOutput = false) const
    {
        if (!copyAndPastableOutput) {
            std::cout << "+---+---+---+---+---+---+---+---+---+\n";
            for (int i = 0; i < 9; ++i) {
                std::cout << "|";
                for (int j = 0; j < 9; ++j) {
                    char sep = '|';
                    if (j < 8 && (j+1)%3 == 0)
                        sep = '#';
                    if (_board[i][j] == 0)
                        std::cout << "   " << sep;
                    else if (_board[i][j] > 9)
                        std::cout << " x " << sep;
                    else
                        std::cout << " " << static_cast<char>('0'+ _board[i][j]) << " " << sep;
                }
                std::cout << "\n";
                if (i >= 8 || (i+1)%3)
                    std::cout << "+---+---+---+---+---+---+---+---+---+\n";
                else
                    std::cout << "+===+===+===+===+===+===+===+===+===+\n";
            }
        }
        else {
            // print the board in a copy-and-pastable way
            std::cout << "SudokuBoard board({\n";
            for (int rowIdx = 0; rowIdx < 9; ++ rowIdx) {
                if (rowIdx > 0 && rowIdx < 8 && rowIdx%3==0)
                    std::cout << "\n";
                std::cout << "{";
                for (int colIdx = 0; colIdx < 9; ++ colIdx) {
                    if (colIdx > 0 && colIdx < 8 && colIdx%3==0)
                        std::cout << " ";
                    std::cout << (int)_board[rowIdx][colIdx];
                    if (colIdx < 8)
                        std::cout << ",";
                }
                std::cout << "},\n";
            }
            std::cout << "});\n";
        }
    }

    uint8_t operator()(int rowIdx, int colIdx) const
    {
        return _board[rowIdx][colIdx];
    }

    // return 0 if the board is not solvable, and a lower limit of the number of
    // solutions if there are some. `nSolsCutoff`, specifies the number of solutions
    // after which we don't care about additional ones anymore...
    int solve(int nSolsCutoff = 1)
    {
        // determine the "first" free position on the board
        int i, j;
        for (i = 0; i < 9; ++i) {
            for (j = 0; j < 9; ++j) {
                if ((*this)(i, j) == 0)
                    goto pos_found;
            }
        }

        return 1; // all fields are occupied!

    pos_found:

        int numFound = 0;

        // try to set one number on that position
        for (int v = 1; v <= 9; ++ v) {
            if (!set(v, i, j))
                // one of the rules is immediately broken by setting the current number. try
                // another one
                continue;

            // one of the rules is broken for another field if the current field is set
            // to v
            if (isAnyDirectlyImpossible()) {
                unset(v, i, j);
                continue;
            }

            // recusively check if the board is still solvable with the current number set
            numFound += solve(nSolsCutoff);
            if (numFound >= nSolsCutoff)
                return numFound;

            unset(v, i, j);
        }

        return numFound;
    }

private:
    void _init()
    {
        for (int i=0; i < 9; ++i) {
            for (int j=0; j < 9; ++j) {
                _board[i][j] = 0;
            }

            _verticalSet[i] = 0;
            _horizontalSet[i] = 0;
            _blockSet[i] = 0;
        }
    }

    void _set(uint8_t num, uint8_t rowIdx, uint8_t colIdx)
    {
        if (num > 0 && num < 10) {
            int mask = 1 << (num - 1);

            // 3x3 block
            int blockIdx = (rowIdx/3) + (colIdx/3)*3;

            _verticalSet[colIdx] |= mask;
            _horizontalSet[rowIdx] |= mask;
            _blockSet[blockIdx] |= mask;
        }

        _board[rowIdx][colIdx] = num;
    }

    uint8_t _board[9][9];
    uint16_t _verticalSet[9];
    uint16_t _horizontalSet[9];
    uint16_t _blockSet[9];
};

// create a random sequence containing all letters from 1 to 9
std::array<std::array<uint8_t, 9>, 1000> shuffles;

bool findChallenge(SudokuBoard& pattern)
{
    // first, ensure that the "fixed" part of the pattern represents a solvable board. if
    // it isn't there's not much point in specifying wild cards
    if (pattern.isAnyDirectlyImpossible())
        return false;

    SudokuBoard tester(pattern);
    for (int rowIdx = 0; rowIdx < 9; ++rowIdx) {
        for (int colIdx = 0; colIdx < 9; colIdx++) {
            if (tester(rowIdx, colIdx) > 9)
                tester.set(0, rowIdx, colIdx);
        }
    }

    int minNumSol = tester.solve(2);
    if (minNumSol == 0)
        return false;
    else if (minNumSol == 1) {
        // the solution is already unique. transfer the numbers of the solution to the
        // remaining wildcards of the pattern and be done with it
        for (int rowIdx = 0; rowIdx < 9; ++rowIdx) {
            for (int colIdx = 0; colIdx < 9; colIdx++) {
                if (pattern(rowIdx, colIdx) > 9) {
                    if (!pattern.set(tester(rowIdx, colIdx), rowIdx,colIdx)) {
                        std::cerr << "Hm, something went wrong!\n";
                        std::abort();
                    }
                }
            }
        }
        pattern.print();
        return true;
    }

    const auto& shuffle = shuffles[rand()%shuffles.size()];
    // fill all "wildcards" (i.e. all positions which need to be initially set by
    // something but where it does not matter by what)
    for (int rowIdx = 0; rowIdx < 9; ++rowIdx) {
        for (int colIdx = 0; colIdx < 9; colIdx++) {
            if (pattern(rowIdx, colIdx) > 9) {
                for (int shuffleIdx = 0; shuffleIdx < 9; ++ shuffleIdx) {
                    if (pattern.set(shuffle[shuffleIdx] + 1, rowIdx, colIdx)) {
                        if (findChallenge(pattern))
                            return true;
                        pattern.unset(10, rowIdx, colIdx);
                    }
                }
                // cannot chose a valid number for the wildcard at (rowIdx, colIdx)
                return false;
            }
        }
    }

    return false;
}

int main()
{
    // initialize the random shuffles. this is a pretty slow operation, so we do not do
    // initialize shuffles inside the solver
    for (auto& shuffle: shuffles) {
        for (unsigned i = 0; i < 9; ++i)
            shuffle[i] = i;
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(shuffle.begin(), shuffle.end(), g);
    }


#define SEL 3
#if SEL == 0
    // blank
    SudokuBoard board({
            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},

            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},

            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},
                       });
#elif SEL == 1
    // "very difficult"
    SudokuBoard board({
            {6,0,0, 0,0,0, 3,0,0},
            {9,0,0, 4,7,0, 1,0,0},
            {7,0,0, 9,5,0, 0,0,0},

            {0,0,0, 3,9,0, 0,0,0},
            {0,3,0, 0,0,0, 0,0,2},
            {0,2,0, 0,0,0, 8,0,4},

            {0,0,6, 2,0,1, 0,0,0},
            {0,0,0, 0,0,0, 6,4,5},
            {4,0,8, 0,0,0, 0,0,0},
                       });
#elif SEL == 2
    // "easy"
    SudokuBoard board({
            {0,0,0, 0,0,0, 4,2,7},
            {6,0,3, 0,0,0, 9,0,0},
            {0,4,0, 5,7,9, 0,0,0},

            {3,5,4, 0,0,0, 0,0,0},
            {0,0,1, 0,8,6, 0,7,4},
            {7,0,0, 0,0,0, 1,9,3},

            {0,0,6, 2,1,0, 0,0,0},
            {8,1,7, 0,6,0, 0,0,2},
            {4,0,0, 7,0,0, 8,6,1},
                       });
#elif SEL == 3
    // "buyacouch" challenge
    uint8_t x = 10;
    SudokuBoard board({
            {x,0,0, x,0,x, x,0,x},
            {x,x,0, x,0,x, 0,x,0},
            {x,x,0, x,x,x, 0,x,0},

            {0,x,0, x,x,0, x,x,x},
            {x,x,x, x,0,0, x,0,x},
            {x,0,x, x,x,0, x,x,x},

            {x,0,x, x,x,0, x,0,x},
            {x,0,x, x,0,0, x,x,x},
            {x,x,x, x,x,0, x,0,x},
                       });
#elif SEL == 4
    // possible solution for the "buyacouch" challenge
    SudokuBoard board({
            {5,0,0, 1,0,2, 8,0,3},
            {7,9,0, 8,0,6, 0,1,0},
            {3,1,0, 7,4,5, 0,9,0},

            {0,7,0, 2,6,0, 3,8,1},
            {4,2,6, 3,0,0, 7,0,9},
            {1,0,3, 5,7,0, 6,2,4},

            {6,0,9, 4,8,0, 1,0,2},
            {8,0,1, 9,0,0, 5,6,7},
            {2,3,7, 6,5,0, 9,0,8},
        });
#else
    #error "no initial board selected"
#endif

#if 1
    std::cout << "pattern:\n";
    board.print();
    std::cout << "challenge with unique solution:\n";
    findChallenge(board);
#else
    const int nMax = 1000;
    auto origBoard = board;
#if 0 // benchmark
    for (int i = 0; i < 100000; ++i) {
        board = origBoard;
        board.solve();
    }
    return 0;
#endif
    int n = board.solve(nMax);
    if (n > 0) {
        if (n == 1)
            std::cout << "unique solution found:\n";
        else if (n < nMax)
            std::cout << n << " solutions found:\n";
        else
            std::cout << "At least " << n << " solutions found:\n";

        std::cout << "original board:\n";
        origBoard.print();
        board.solve(/*nMax=*/1);
        std::cout << "first solution:\n";
        board.print();
    }
    else
        std::cout << "not solvable\n";
#endif

    return 0;
}
