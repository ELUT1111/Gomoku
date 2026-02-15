#include "humanplayer.h"
#include <qDebug>

HumanPlayer::HumanPlayer(QObject *parent, ChessType chessType)
    : AbstractPlayer{parent,chessType}
{}

void HumanPlayer::slot_onMouseClicked(int x, int y)
{
    qDebug()<<"[humanPlayer] 接收点击请求: "<<x<<","<<y;
    emit signal_tryPlaceChess(x, y, myChessType);
}
