#include "aiplayer.h"

AIPlayer::AIPlayer(QObject *parent, ChessType chessType)
    : AbstractPlayer{parent, chessType}
{}

void AIPlayer::slot_onMouseClicked(int x, int y)
{
    return;
}
