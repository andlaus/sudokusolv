// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
#include <algorithm>
#include <array>
#include <cassert>
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
                _assign(*jIt, i, j);
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

                // no number can be placed on the current field
                if (possibleSet(rowIdx, colIdx) == 0)
                    return true;
            }
        }

        // a number could be placed on every free field
        return false;
    }

    // returns the set of all possible numbers at a given position
    uint16_t possibleSet(uint8_t rowIdx, uint8_t colIdx) const
    {
        int blockIdx = (rowIdx/3) + (colIdx/3)*3;
        return 0x1ff&(~_horizontalSet[rowIdx])&(~_verticalSet[colIdx])&(~_blockSet[blockIdx]);
    }

    // return true iff each field in a given row, column and block can still be assigned
    // a number
    bool scanPosition(uint8_t rowIdx, uint8_t colIdx) const
    {
        // scan horizontal and vertical lines
        for (int i = 0; i < 9; ++i) {
            if ((_board[rowIdx][i] == 0 && possibleSet(rowIdx, i) == 0) ||
                (_board[i][colIdx] == 0 && possibleSet(i, colIdx) == 0))
                return false;
        }

        // scan 3x3 block
        int row0Idx = rowIdx - rowIdx%3;
        int col0Idx = colIdx - colIdx%3;
        for (int rowIdx = row0Idx; rowIdx < row0Idx+3; ++rowIdx) {
            for (int colIdx = col0Idx; colIdx < col0Idx+3; ++colIdx) {
                if ((_board[rowIdx][colIdx] == 0 && possibleSet(rowIdx, colIdx) == 0))
                    return false;
            }
        }

        return true;
    }

    bool assign(uint8_t num, uint8_t rowIdx, uint8_t colIdx)
    {
        if (num > 0 && num < 10) {
            int mask = 1 << (num - 1);
            if (!(mask&possibleSet(rowIdx, colIdx)))
                return false;

            int blockIdx = (rowIdx/3) + (colIdx/3)*3;
            _verticalSet[colIdx] |= mask;
            _horizontalSet[rowIdx] |= mask;
            _blockSet[blockIdx] |= mask;
        }
        _board[rowIdx][colIdx] = num;

        return true;
    }

    void unassign(uint8_t new_val, uint8_t rowIdx, uint8_t colIdx)
    {
        uint8_t cur_val = _board[rowIdx][colIdx];

        if (cur_val > 0 && cur_val < 10) {
            int mask = ~(1 << (cur_val - 1));

            int blockIdx = (rowIdx/3) + (colIdx/3)*3;

            _verticalSet[colIdx] &= mask;
            _horizontalSet[rowIdx] &= mask;
            _blockSet[blockIdx] &= mask;
        }

        assert(new_val < 1 || new_val > 9);
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
    int solve(int nSolsCutoff = 1, SudokuBoard* solution = nullptr)
    {
        // determine the "first" free position on the board
        int i, j;
        uint8_t v0;
        for (i = 0; i < 9; ++i) {
            for (j = 0; j < 9; ++j) {
                if (_board[i][j] < 1 || _board[i][j] > 9) {
                    v0 = _board[i][j];
                    goto pos_found;
                }
            }
        }

        if (solution)
            *solution = *this;

        return 1; // all fields are occupied!

    pos_found:

        int numFound = 0;

        // try to set one number on that position
        for (int v = 1; v <= 9; ++ v) {
            if (!assign(v, i, j))
                // one of the rules is immediately broken by setting the current number. try
                // another one
                continue;

            if (!scanPosition(i, j)) {
                // one of the rules is broken for another field if the current field is
                // set to v
                unassign(v0, i, j);
                continue;
            }

            // recusively check if the board is still solvable with the current number set
            numFound += solve(nSolsCutoff, solution);
            if (numFound >= nSolsCutoff)
                return numFound;

            unassign(v0, i, j);
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

    void _assign(uint8_t num, uint8_t rowIdx, uint8_t colIdx)
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

bool findChallenge(SudokuBoard& pattern, bool doPrint = true)
{
    // first, ensure that the "fixed" part of the pattern represents a solvable board. if
    // it isn't there's not much point in specifying wild cards
    if (pattern.isAnyDirectlyImpossible())
        return false;

    SudokuBoard tester(pattern), sol;
    for (int rowIdx = 0; rowIdx < 9; ++rowIdx) {
        for (int colIdx = 0; colIdx < 9; colIdx++) {
            if (tester(rowIdx, colIdx) > 9)
                tester.assign(0, rowIdx, colIdx);
        }
    }

    int minNumSol = tester.solve(/*nCutoff=*/2, &sol);
    if (minNumSol == 0)
        return false;
    else if (minNumSol == 1) {
        // the solution is already unique. transfer the numbers of the solution to the
        // remaining wildcards of the pattern and be done with it
        for (int rowIdx = 0; rowIdx < 9; ++rowIdx) {
            for (int colIdx = 0; colIdx < 9; colIdx++) {
                if (pattern(rowIdx, colIdx) > 9) {
                    if (!pattern.assign(sol(rowIdx, colIdx), rowIdx,colIdx) ||
                        !pattern.scanPosition(rowIdx, colIdx)) {
                        std::cerr << "Hm, something went wrong!\n";
                        std::abort();
                    }
                }
            }
        }
        if (doPrint)
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
                    if (pattern.assign(shuffle[shuffleIdx] + 1, rowIdx, colIdx)) {
                        if (pattern.scanPosition(rowIdx, colIdx) &&
                            findChallenge(pattern, doPrint))
                            return true;
                        pattern.unassign(10, rowIdx, colIdx);
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


//#define BENCHMARK_NUM_REPETITIONS (1000)
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
#define FIND_CHALLENGE_MODE 1
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

#ifdef FIND_CHALLENGE_MODE
#ifdef BENCHMARK_NUM_REPETITIONS
    std::cout << "find challenge benchmark: " << BENCHMARK_NUM_REPETITIONS << " repetitions\n";
    std::cout << "pattern:\n";
    board.print();
    auto origBoard = board;
    for (int i = 0; i < BENCHMARK_NUM_REPETITIONS; ++i) {
        board = origBoard;
        findChallenge(board, /*doPrint=*/false);
    }
    return 0;
#endif
    std::cout << "finding challenge for pattern:\n";
    board.print();
    std::cout << "challenge with unique solution:\n";
    findChallenge(board);
#else
    auto origBoard = board;
#ifdef BENCHMARK_NUM_REPETITIONS
    std::cout << "solver benchmark: " << BENCHMARK_NUM_REPETITIONS << " repetitions\n";
    for (int i = 0; i < BENCHMARK_NUM_REPETITIONS; ++i) {
        board = origBoard;
        board.solve();
    }
    return 0;
#endif

    const int nMax = 1000;
    SudokuBoard sol;
    int n = board.solve(nMax, &sol);
    if (n > 0) {
        std::cout << "solving board:\n";
        origBoard.print();

        if (n == 1)
            std::cout << "unique solution found:\n";
        else if (n < nMax)
            std::cout << n << " solutions found:\n";
        else
            std::cout << "At least " << n << " solutions found:\n";

        std::cout << "possible solution:\n";
        sol.print();
    }
    else
        std::cout << "not solvable\n";
#endif

    return 0;
}
