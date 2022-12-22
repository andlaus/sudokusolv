// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
#include <cstdint>
#include <iostream>
#include <iomanip>

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

    bool set(uint8_t num, uint8_t x, uint8_t y)
    {
        int mask = 1 << (num - 1);

        // horizontal line
        if (_horSet[y] & mask)
            return false;
        // vertical line
        else if (_vertSet[x] & mask)
            return false;

        // 3x3 block
        int k = (x/3) + (y/3)*3;
        if (_blockSet[k] & mask)
            return false;

        _board[x][y] = num;
        _horSet[y] |= mask;
        _vertSet[x] |= mask;
        _blockSet[k] |= mask;

        return true;
    }

    void unset(uint8_t num, uint8_t x, uint8_t y)
    {
        int mask = ~(1 << (num - 1));

        int k = (x/3) + (y/3)*3;

        _board[x][y] = 0;
        _horSet[y] &= mask;
        _vertSet[x] &= mask;
        _blockSet[k] &= mask;
    }

    void print() const
    {
        std::cout << "+---+---+---+---+---+---+---+---+---+\n";
        for (int i = 0; i < 9; ++i) {
            std::cout << "|";
            for (int j = 0; j < 9; ++j) {
                char sep = '|';
                if (j < 8 && (j+1)%3 == 0)
                    sep = '#';
                if (_board[i][j] == 0)
                    std::cout << "   " << sep;
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

    uint8_t operator()(int x, int y) const
    {
        return _board[x][y];
    }

    // return 0 if the board is not solvable, 1 if it is uniquely solvable and 2 if more than
    // one solution exists
    int solve(int maxSols = 1)
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

            // recusively check if the board is still solvable with the current number set
            numFound += solve(maxSols);
            if (numFound >= maxSols)
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

            _horSet[i] = 0;
            _vertSet[i] = 0;
            _blockSet[i] = 0;
        }
    }

    void _set(uint8_t num, uint8_t x, uint8_t y)
    {
        int mask = 1 << (num - 1);

        // 3x3 block
        int k = (x/3) + (y/3)*3;

        _board[x][y] = num;
        _horSet[y] |= mask;
        _vertSet[x] |= mask;
        _blockSet[k] |= mask;
    }

    uint8_t _board[9][9];
    uint16_t _horSet[9];
    uint16_t _vertSet[9];
    uint16_t _blockSet[9];
};

void findChallenge(SudokuBoard& pattern)
{
}

int main()
{
#define SEL 2
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
    int x = 0xa;
    SudokuBoard board({
            {x,x,0, x,0,x, x,0,x},
            {x,x,x, x,x,x, 0,x,0},
            {x,x,x, x,x,x, 0,x,0},

            {0,x,0, x,x,x, x,x,x},
            {x,x,x, x,0,0, x,0,x},
            {x,0,x, x,x,x, x,x,x},

            {x,0,x, x,x,x, x,0,x},
            {x,0,x, x,0,0, x,x,x},
            {x,x,x, x,x,x, x,0,x},
                       });
#else
    #error "no initial board selected"
#endif

    auto origBoard = board;
    const int nMax = 1000;
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

    return 0;
}
