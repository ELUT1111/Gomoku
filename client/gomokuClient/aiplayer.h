#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "abstractplayer.h"

class AIPlayer : public AbstractPlayer
{
public:
    explicit AIPlayer(QObject *parent = nullptr,ChessType chessType = ChessType::BLACK);

public slots:
    void slot_onMouseClicked(int x,int y) override;
};

#endif // AIPLAYER_H
