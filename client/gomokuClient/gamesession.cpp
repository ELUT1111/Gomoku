#include "gamesession.h"
#include <qDebug>

GameSession* GameSession::gameSession = nullptr;
QMutex GameSession::mutex;

GameSession::GameSession(QObject *parent)
    : QObject{parent}
{
    initBoard();
    initPlayer();
    initConnect();
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
    player1 = new AbstractPlayer(this,ChessType::BLACK);
    player2 = new AbstractPlayer(this,ChessType::WHITE);
    currentPlayer = player1;
    lastPlayer = player2;
}

void GameSession::initConnect()
{
    connect(player1,&AbstractPlayer::signal_tryPlaceChess,this,&GameSession::slot_placeChess);
    connect(player2,&AbstractPlayer::signal_tryPlaceChess,this,&GameSession::slot_placeChess);
}

void GameSession::resetTurn()
{
    currentPlayer = player1;
    lastPlayer = player2;
    emit signal_switchTurn();
}

bool GameSession::checkWin(int x, int y, ChessType chessType)
{
    for(int i=1;i<4;i++)
    {
        if(boardData->numInRow(x,y,chessType,i)>=5)
        {
            return true;
        }
    }
    return false;
}

void GameSession::slot_placeChess(int x, int y, ChessType chessType)
{
    qDebug()<<"[session] 落子: "<<x<<","<<y;
    emit signal_drawChess(x, y ,chessType);
    boardData->setChess(x, y, chessType);
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
    resetTurn();
}

void GameSession::slot_handleUndo()
{
    qDebug()<<"[session] 处理悔棋";
    if(chessHistory.isEmpty()) return;
    ChessHistory last = chessHistory.takeLast();
    boardData->setChess(last.pos.x(),last.pos.y(),ChessType::EMPTY);
    std::swap(currentPlayer,lastPlayer);
    emit signal_switchTurn();
}
