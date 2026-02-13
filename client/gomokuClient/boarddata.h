#ifndef BOARDDATA_H
#define BOARDDATA_H

#include <QObject>
#include <EnumType.h>
#include <QReadWriteLock>

class BoardData : public QObject
{
    Q_OBJECT
public:
    explicit BoardData(QObject *parent = nullptr);

    ChessType getChess(int x, int y) const;
    // 批量复制棋盘
    void getBoard(int newBoard[BOARD_SIZE][BOARD_SIZE]) const;
    // 检查位置是否合法
    bool isPosValid(int x, int y) const;
    bool isPosEmpty(int x, int y) const;
    // 某方向连子数量
    int numInRow(int x,int y,ChessType chessType,int dir);

    // 写接口
    // 设置指定位置的棋子
    void setChess(int x, int y, ChessType chessType);
    // 清空棋盘
    void clear();
private:
    // 棋盘数组
    int board[BOARD_SIZE][BOARD_SIZE] = {0};
    // 多读单写锁
    mutable QReadWriteLock rwLock;
signals:
};

using BoardDataPtr = QSharedPointer<BoardData>;

#endif // BOARDDATA_H
