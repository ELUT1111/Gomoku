#include "abstractplayer.h"

#include <qDebug>
AbstractPlayer::AbstractPlayer(QObject *parent, ChessType chessType)
    : QObject{parent}, myChessType(chessType)
{}

void AbstractPlayer::slot_onMouseClicked(int x, int y)
{
    return;
}

void AbstractPlayer::slot_myTurn()
{
    return;
}
