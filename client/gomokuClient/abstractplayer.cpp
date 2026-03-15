#include "abstractplayer.h"

#include <qDebug>
AbstractPlayer::AbstractPlayer(QObject *parent, ChessType chessType)
    : QObject{parent}, myChessType(chessType)
{}

ChessType AbstractPlayer::getMyChessType() const
{
    return myChessType;
}

void AbstractPlayer::setMyChessType(ChessType newMyChessType)
{
    myChessType = newMyChessType;
}

void AbstractPlayer::slot_onMouseClicked(int x, int y)
{
    return;
}

void AbstractPlayer::slot_myTurn()
{
    return;
}
