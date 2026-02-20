#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "abstractplayer.h"

class AIPlayer : public AbstractPlayer
{
    Q_OBJECT
public:
    explicit AIPlayer(QObject *parent = nullptr,ChessType chessType = ChessType::BLACK, AIType difficutify = AIType::EASY);
    AIType getDifficutify() const;
    void setDifficutify(AIType newDifficutify);

    void startThinking();
    void cancelThinking();
    QPoint nextPos();
    QPoint nextPosForEasy();
    QPoint nextPosForNormal();
    QPoint nextPosForHard();
    QPoint randomEmptyPos();

protected:
    AIType difficutify;
    int isThinking = 0;
public slots:
    void slot_onMouseClicked(int x,int y) override;

    void slot_myTurn() override;
};

#endif // AIPLAYER_H
