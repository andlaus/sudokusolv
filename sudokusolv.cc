#include <cstdint>
#include <iostream>
#include <iomanip>

class SudokuBoard
{
public:
    SudokuBoard()
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
        std::cout << "\n";
        std::cout << "+---+---+---+---+---+---+---+---+---+\n";
        for (int i = 0; i < 9; ++i) {
            std::cout << "|";
            for (int j = 0; j < 9; ++j) {
                if (_board[i][j] == 0)
                    std::cout << "   |";
                else
                    std::cout << " " << static_cast<char>('0'+ _board[i][j]) << " |";
            }
            std::cout << "\n";
            std::cout << "+---+---+---+---+---+---+---+---+---+\n";
        }
    }

    uint8_t operator()(int x, int y) const
    {
        return _board[x][y];
    }

private:
    uint8_t _board[9][9];
    uint16_t _horSet[9];
    uint16_t _vertSet[9];
    uint16_t _blockSet[9];
};

// return 0 if the board is not solvable, 1 if it is uniquely solvable and 2 if more than
// one solution exists
int solve(SudokuBoard& board, int maxSols = 1)
{
    // determine the "first" free position on the board
    int i, j;
    for (i = 0; i < 9; ++i) {
        for (j = 0; j < 9; ++j) {
            if (board(i, j) == 0)
                goto pos_found;
        }
    }

    return 1; // all fields are occupied!

pos_found:

    int numFound = 0;

    // try to set one number on that position
    for (int v = 1; v <= 9; ++ v) {
        if (!board.set(v, i, j))
            // one of the rules is immediately broken by setting the current number. try
            // another one
            continue;

        // recusively check if the board is still solvable with the current number set
        numFound += solve(board, maxSols);
        if (numFound >= maxSols)
            return numFound;

        board.unset(v, i, j);
    }

    return numFound;
}

int main()
{
    SudokuBoard board;

    const int maxSols = 1000;
    int numSols = solve(board, maxSols);
    if (numSols > 0) {
        if (numSols >= maxSols)
            std::cout << "solvable, at least " << numSols << " solutions.\n";
        else
            std::cout << "solvable, " << numSols << " solutions.\n";
        std::cout << "First found solution:\n";
        solve(board, /*maxSols=*/1);
        board.print();
    }
    else
        std::cout << "not solvable\n";

    return 0;
}
