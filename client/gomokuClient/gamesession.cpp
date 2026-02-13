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
    qDebug()<<"[session]落子: "<<x<<","<<y;
    emit signal_drawChess(x, y ,chessType);
    boardData->setChess(x, y, chessType);
    std::swap(currentPlayer,lastPlayer);
    emit signal_switchTurn();
}
