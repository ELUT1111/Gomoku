#ifndef ENUMTYPE_H
#define ENUMTYPE_H

#include <QMetaType>

// 棋盘大小
const int BOARD_SIZE = 15;
//方向数组
const int DIRS[8][2] = {{0,1},{1,0},{1,1},{-1,1},{0,-1},{-1,0},{-1,-1},{1,-1}};

enum class ChessType
{
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

enum class AIType
{
    EASY =1,
    NORMAL = 2,
    HARD = 3
};

enum class GamemodeType
{
    OFFLINE =0,
    ONLINE =1
};

Q_DECLARE_METATYPE(ChessType)
Q_DECLARE_METATYPE(AIType)
Q_DECLARE_METATYPE(GamemodeType)

#endif // ENUMTYPE_H
