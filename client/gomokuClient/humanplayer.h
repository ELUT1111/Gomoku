#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "abstractplayer.h"

class HumanPlayer : public AbstractPlayer
{
public:
    explicit HumanPlayer(QObject *parent = nullptr);
};

#endif // HUMANPLAYER_H
