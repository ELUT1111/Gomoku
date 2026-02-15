#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "abstractplayer.h"

class HumanPlayer : public AbstractPlayer
{
public:
    explicit HumanPlayer(QObject *parent = nullptr , ChessType chessType = ChessType::BLACK);

    // AbstractPlayer interface
public slots:
    void slot_onMouseClicked(int x, int y) override;
};

#endif // HUMANPLAYER_H
