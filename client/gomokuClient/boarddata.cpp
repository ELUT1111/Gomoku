#include "boarddata.h"

BoardData::BoardData(QObject *parent)
    : QObject{parent}
{
    clear();
}

ChessType BoardData::getChess(int x, int y) const
{
    QReadLocker locker(&rwLock); // 读锁
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return ChessType::EMPTY;
    }
    return (ChessType)board[x][y];
}

void BoardData::getBoard(int newBoard[BOARD_SIZE][BOARD_SIZE]) const
{
    QReadLocker locker(&rwLock);
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            newBoard[x][y] = board[x][y];
        }
    }
}

bool BoardData::isPosValid(int x, int y) const
{
    QReadLocker locker(&rwLock);
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

bool BoardData::isPosEmpty(int x, int y) const
{
    return isPosValid(x, y) && getChess(x,y) == ChessType::EMPTY;
}

void BoardData::setChess(int x, int y, ChessType chessType)
{
    QWriteLocker locker(&rwLock); // 写锁
    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
        board[x][y] = static_cast<int>(chessType);
    }
}
int BoardData::numInRow(int x, int y, ChessType chessType, int dir)
{
    if(getChess(x,y) != chessType) return 0;
    int num = 0;
    int x1=x,y1=y;
    while(isPosValid(x,y)&&getChess(x, y) == chessType)
    {
        num++;
        x+=DIRS[dir][0];
        y+=DIRS[dir][1];
    }
    x=x1 - DIRS[dir][0];
    y=y1 - DIRS[dir][1];
    while(isPosValid(x,y)&&getChess(x, y) == chessType)
    {
        num++;
        x -= DIRS[dir][0];
        y -= DIRS[dir][1];
    }
    return num;
}
void BoardData::clear()
{
    QWriteLocker locker(&rwLock);
    memset(board, 0, sizeof(board));
}

int BoardData::getChessNumbers() const
{
    QReadLocker locker(&rwLock);
    return chessNumbers;
}

void BoardData::setChessNumbers(int newChessNumbers)
{
    QWriteLocker locker(&rwLock);
    chessNumbers = newChessNumbers;
}
