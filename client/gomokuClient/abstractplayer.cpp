#include "abstractplayer.h"

#include <qDebug>
AbstractPlayer::AbstractPlayer(QObject *parent, ChessType chessType)
    : QObject{parent}, myChessType(chessType)
{}

void AbstractPlayer::slot_onMouseClicked(int x, int y)
{
    qDebug()<<"[currentPlayer]接收点击请求: "<<x<<","<<y;
    emit signal_tryPlaceChess(x, y, myChessType);
}
