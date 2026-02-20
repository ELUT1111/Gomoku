#ifndef SHAREDTYPE_H
#define SHAREDTYPE_H

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
    OFFLINE_FREE = 0,
    OFFLINE_AI = 1,
    ONLINE = 2
};

namespace EnumUtils{
    inline ChessType oppo(ChessType ct)
    {
        if(ct == ChessType::BLACK) return ChessType::WHITE;
        else return ChessType::BLACK;
    }
}

Q_DECLARE_METATYPE(ChessType)
Q_DECLARE_METATYPE(AIType)
Q_DECLARE_METATYPE(GamemodeType)

#endif // SHAREDTYPE_H
