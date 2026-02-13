#ifndef ABSTRACTPLAYER_H
#define ABSTRACTPLAYER_H

#include <EnumType.h>
#include <QObject>

class AbstractPlayer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPlayer(QObject *parent = nullptr, ChessType chessType = ChessType::BLACK);
private:
    ChessType myChessType;
signals:
    void signal_tryPlaceChess(int x,int y,ChessType chessType);
public slots:
    void slot_onMouseClicked(int x,int y);

};

#endif // ABSTRACTPLAYER_H
