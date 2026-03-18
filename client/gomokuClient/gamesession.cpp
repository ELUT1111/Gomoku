#include "gamesession.h"
#include <AIPlayer.h>
#include <HumanPlayer.h>
#include <qDebug>

GameSession* GameSession::gameSession = nullptr;
QMutex GameSession::mutex;

GameSession::GameSession(QObject *parent)
    : QObject{parent}
{
    initBoard();
    // initPlayer();
    // initConnect();
}

GameSession *GameSession::instance()
{
    if(!gameSession)
    {
        QMutexLocker locker(&mutex);
        gameSession = new GameSession();
        return gameSession;
    }
    return gameSession;
}


void GameSession::initBoard()
{
    boardData = QSharedPointer<BoardData>::create(this);
    //board = QSharedPointer<BoardData>(new BoardData(this));
}

void GameSession::initPlayer()
{
    if(player1) delete player1;
    if(player2) delete player2;
    player1 = nullptr;
    player2 = nullptr;
    m_onlineBlackPlayer = nullptr;
    m_onlineWhitePlayer = nullptr;
    if(gamemode == GamemodeType::OFFLINE_FREE)
    {
        player1 = new HumanPlayer(this,ChessType::BLACK);
        player2 = new HumanPlayer(this,ChessType::WHITE);
        currentPlayer = player1;
        lastPlayer = player2;
    }else if(gamemode == GamemodeType::OFFLINE_AI_EASY)
    {
        player1 = new HumanPlayer(this,ChessType::BLACK);
        player2 = new AIPlayer(this,ChessType::WHITE,AIType::EASY);
        currentPlayer = player1;
        lastPlayer = player2;
    }else if(gamemode == GamemodeType::OFFLINE_AI_NORMAL)
    {
        player1 = new HumanPlayer(this,ChessType::BLACK);
        player2 = new AIPlayer(this,ChessType::WHITE,AIType::NORMAL);
        currentPlayer = player1;
        lastPlayer = player2;
    }else if(gamemode == GamemodeType::OFFLINE_AI_HARD)
    {
        player1 = new HumanPlayer(this,ChessType::BLACK);
        player2 = new AIPlayer(this,ChessType::WHITE,AIType::HARD);
        currentPlayer = player1;
        lastPlayer = player2;
    }
    else if(gamemode == GamemodeType::ONLINE)
    {
        m_onlineBlackPlayer = new OnlinePlayer(this, ChessType::BLACK);
        m_onlineWhitePlayer = new OnlinePlayer(this, ChessType::WHITE);

        // 绑定网络层的对手落子信号→在线玩家处理
        NetworkManager& netMgr = NetworkManager::instance();
        connect(&netMgr, &NetworkManager::moveReceived, this, &GameSession::slot_handleOpponentMove);
        // 绑定网络层的游戏结束信号→会话处理
        connect(&netMgr, &NetworkManager::sig_gameOverReceived, this, &GameSession::slot_handleOnlineGameOver);

        // 绑定网络层的错误信号→会话转发
        connect(&netMgr, &NetworkManager::sig_errorReceived, this, &GameSession::sig_onlineError, Qt::UniqueConnection);
    }

}

void GameSession::initConnect()
{
    if( gamemode!= GamemodeType::ONLINE)
    {
        connect(player1,&AbstractPlayer::signal_tryPlaceChess,this,&GameSession::slot_placeChess,Qt::UniqueConnection);
        connect(player2,&AbstractPlayer::signal_tryPlaceChess,this,&GameSession::slot_placeChess,Qt::UniqueConnection);
    }
}

void GameSession::resetTurn()
{
    if(gamemode == GamemodeType::ONLINE) return;

    currentPlayer = player1;
    lastPlayer = player2;
    emit signal_switchTurn();
}

bool GameSession::checkWin(int x, int y, ChessType chessType)
{
    if(gamemode == GamemodeType::ONLINE) return false;

    for(int i=0;i<4;i++)
    {
        //qDebug()<<boardData->numInRow(x,y,chessType,i);
        if(boardData->numInRow(x,y,chessType,i)>=5)
        {
            return true;
        }
    }
    return false;
}

bool GameSession::isAIMode()
{
    return gamemode == GamemodeType::OFFLINE_AI_EASY ||
           gamemode == GamemodeType::OFFLINE_AI_NORMAL ||
           gamemode == GamemodeType::OFFLINE_AI_HARD;
}

void GameSession::slot_changeGamemode(GamemodeType gamemode)
{
    qDebug()<<"[session] 当前模式:"<<int(gamemode);
    this->gamemode = gamemode;
    initPlayer();
    initConnect();
    slot_resetGame();
}

void GameSession::slot_placeChess(int x, int y, ChessType chessType)
{


    qDebug()<<"[session] 落子: "<<x<<","<<y;
    emit signal_drawChess(x, y ,chessType);
    boardData->setChess(x, y, chessType);

    if(gamemode == GamemodeType::ONLINE)
    {
        return;
    }

    chessHistory.append(ChessHistory(QPoint(x,y),currentPlayer));
    if(checkWin(x,y,chessType))
    {
        qDebug()<<"[session] 游戏结算";
        emit signal_playerWin(currentPlayer);
    }
    else
    {
        //交换回合
        std::swap(currentPlayer,lastPlayer);
        emit signal_switchTurn();
    }
}

void GameSession::slot_resetGame()
{
    qDebug()<<"[session] 重置棋盘";
    boardData->clear();
    chessHistory.clear();

    if(gamemode == GamemodeType::ONLINE) return;

    resetTurn();
}

void GameSession::slot_handleUndo()
{
    if(gamemode == GamemodeType::ONLINE) return;

    qDebug()<<"[session] 处理悔棋";
    if(chessHistory.isEmpty()) return;
    ChessHistory last = chessHistory.takeLast();
    if(gamemode == GamemodeType::OFFLINE_FREE)
    {
        boardData->setChess(last.pos.x(),last.pos.y(),ChessType::EMPTY);
        std::swap(currentPlayer,lastPlayer);
        emit signal_switchTurn();
    }
    else if(gamemode == GamemodeType::OFFLINE_AI_EASY)
    {
        if(chessHistory.isEmpty()) return;
        ChessHistory last2 = chessHistory.takeLast();
        boardData->setChess(last.pos.x(),last.pos.y(),ChessType::EMPTY);
        boardData->setChess(last2.pos.x(),last2.pos.y(),ChessType::EMPTY);
    }

}
void GameSession::setOnlinePlayerTag(ChessType chessType, const QString& tag)
{
    if(chessType == ChessType::BLACK && m_onlineBlackPlayer)
    {
        m_onlineBlackPlayer->setOnlinePlayerTag(tag);
    }else if(chessType == ChessType::WHITE && m_onlineWhitePlayer)
    {
        m_onlineWhitePlayer->setOnlinePlayerTag(tag);
    }
}

// 获取在线玩家
OnlinePlayer* GameSession::getOnlinePlayer(ChessType chessType)
{
    return (chessType == ChessType::BLACK) ? m_onlineBlackPlayer : m_onlineWhitePlayer;
}

// 处理对手落子→转发给在线玩家
void GameSession::slot_handleOpponentMove(int x, int y, int color)
{
    if(gamemode != GamemodeType::ONLINE) return;
    // 转发给在线玩家处理绘子
    if(color == 1) // 对手黑方→己方白方处理
    {
        m_onlineWhitePlayer->slot_onOpponentMoveReceived(x, y, color);
    }else // 对手白方→己方黑方处理
    {
        m_onlineBlackPlayer->slot_onOpponentMoveReceived(x, y, color);
    }
}

// 处理在线游戏结束
void GameSession::slot_handleOnlineGameOver(QString msg)
{
    if(gamemode != GamemodeType::ONLINE) return;
    qDebug() << "[GameSession] 在线游戏结束：" << msg;
    emit sig_onlineGameOver(msg);
    // 重置游戏
    slot_resetGame();
}
