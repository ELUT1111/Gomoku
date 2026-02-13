#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "boarddata.h"
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <AbstractPlayer.h>

class GameSession : public QObject
{
    /* 单例会话类，统一处理游戏逻辑 */
    Q_OBJECT
    static GameSession* gameSession;
    static QMutex mutex;
public:
    explicit GameSession(QObject *parent = nullptr);

    static GameSession* instance();

    void initBoard();
    void initPlayer();
    void initConnect();

    bool checkWin(int x,int y,ChessType chessType);

    BoardDataPtr getBoardData() const { return boardData; }
public:
    AbstractPlayer* player1;
    AbstractPlayer* player2;
    AbstractPlayer* currentPlayer; // 当前客户端真人玩家，接受UI的输入转发
    AbstractPlayer* lastPlayer; // 离线自由练习模式下使用
    BoardDataPtr boardData;


signals:
    void signal_drawChess(int x, int y, ChessType chessType);
    void signal_switchTurn();
public slots:
    void slot_placeChess(int x,int y, ChessType chessType);
};

#endif // GAMESESSION_H
