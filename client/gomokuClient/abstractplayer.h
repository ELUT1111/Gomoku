#ifndef ABSTRACTPLAYER_H
#define ABSTRACTPLAYER_H

#include <EnumType.h>
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

};

#endif // ABSTRACTPLAYER_H
