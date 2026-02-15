#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "boarddata.h"
#include <QObject>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <AbstractPlayer.h>
#include <QPoint>

struct ChessHistory
{
    int index; // 1-based
    QPoint pos;
    AbstractPlayer* player;

    ChessHistory(QPoint pos,AbstractPlayer *player,int index = 0):
        index(index),pos(pos),player(player)
    {

    }
};

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
    void resetTurn();

    bool checkWin(int x,int y,ChessType chessType);

    BoardDataPtr getBoardData() const { return boardData; }
public:
    AbstractPlayer* player1;
    AbstractPlayer* player2;
    AbstractPlayer* currentPlayer; // 当前客户端真人玩家，接受UI的输入转发
    AbstractPlayer* lastPlayer; // 离线自由练习模式下使用
    BoardDataPtr boardData;
    QList<ChessHistory> chessHistory;


signals:
    void signal_drawChess(int x, int y, ChessType chessType);
    void signal_switchTurn();
    void signal_playerWin(AbstractPlayer* player);
public slots:
    void slot_placeChess(int x,int y, ChessType chessType);
    void slot_resetGame();
    void slot_handleUndo();
};

#endif // GAMESESSION_H
