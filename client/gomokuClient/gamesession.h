#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "boarddata.h"
#include <QObject>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <AbstractPlayer.h>
#include "OnlinePlayer.h"
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
    bool isAIMode();

    void setOnlinePlayerTag(ChessType chessType, const QString& tag);
    // 在线模式：获取当前在线玩家
    OnlinePlayer* getOnlinePlayer(ChessType chessType);

    BoardDataPtr getBoardData() const { return boardData; }
public:
    GamemodeType gamemode;
    AbstractPlayer* player1;
    AbstractPlayer* player2;
    AbstractPlayer* currentPlayer; // 当前客户端真人玩家，接受UI的输入转发
    AbstractPlayer* lastPlayer; // 离线自由练习模式下使用
    OnlinePlayer* m_onlineBlackPlayer = nullptr; // 在线黑方
    OnlinePlayer* m_onlineWhitePlayer = nullptr; // 在线白方
    BoardDataPtr boardData;
    QList<ChessHistory> chessHistory;


signals:
    void signal_drawChess(int x, int y, ChessType chessType);
    void signal_switchTurn();
    void signal_playerWin(AbstractPlayer* player);
    void sig_onlineGameOver(QString msg); // 在线游戏结束
    void sig_onlineError(QString msg);    // 在线错误提示
    void sig_playerReadyChanged(bool currentState,QString msg);
public slots:
    void slot_changeGamemode(GamemodeType gamemode);
    void slot_placeChess(int x,int y, ChessType chessType);
    void slot_resetGame();
    void slot_handleUndo();

    void slot_handleOpponentMove(int x, int y, int color);
    // 处理在线游戏结束
    void slot_handleOnlineGameOver(QString msg);
};

#endif // GAMESESSION_H
