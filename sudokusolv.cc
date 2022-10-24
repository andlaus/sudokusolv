#include <cstdint>
#include <iostream>

class SudokuBoard
{
public:
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

bool solve(SudokuBoard& board, int print = 0)
{
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (print)
                std::cout << "testing ("<<i<<", "<<j<<")\n";
            if (board(i, j) != 0) {
                std::cout << "cont\n";
                continue;
            }

            for (int v=1; v <= 9; ++ v) {
                if (!board.set(v, i, j)) {
                    board.unset(v, i, j);
                    continue;
                }

                if (solve(board))
                    return true;

                board.unset(v, i, j);
            }
        }
    }

    return false;
}

int main()
{
    SudokuBoard board;

    if (solve(board, /*print=*/true))
        std::cout << "solvable\n";
    else
        std::cout << "not solvable\n";

    return 0;
}
