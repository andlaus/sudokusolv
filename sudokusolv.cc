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

bool solve(SudokuBoard& board, int print = 0)
{
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (board(i, j) != 0) {
                continue;
            }

            for (int v = 1; v <= 9; ++ v) {
                if (!board.set(v, i, j))
                    // one of the rules is immediately broken. try another number
                    continue;

                if (solve(board))
                    return true;

                board.unset(v, i, j);
            }

            // if we cannot place any number on the tile that leads to a solution, we
            // give up
            return false;
        }
    }

    return true;
}

int main()
{
    SudokuBoard board;

    if (solve(board, /*print=*/true)) {
        std::cout << "solvable:\n";
        board.print();
    }
    else
        std::cout << "not solvable\n";

    return 0;
}
