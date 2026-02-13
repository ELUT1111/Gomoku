#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "abstractplayer.h"

class AIPlayer : public AbstractPlayer
{
public:
    explicit AIPlayer(QObject *parent = nullptr);
};

#endif // AIPLAYER_H
