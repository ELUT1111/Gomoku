#ifndef ABSTRACTPLAYER_H
#define ABSTRACTPLAYER_H

#include <SharedType.h>
#include <QObject>

class AbstractPlayer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPlayer(QObject *parent = nullptr, ChessType chessType = ChessType::BLACK);
protected:
    ChessType myChessType;
signals:
    void signal_tryPlaceChess(int x,int y,ChessType chessType);
public slots:
    virtual void slot_onMouseClicked(int x,int y);
    virtual void slot_myTurn();

};

#endif // ABSTRACTPLAYER_H
