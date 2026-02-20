#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "SharedType.h"
#include "abstractplayer.h"
#include "gamesession.h"
#include "boarddata.h"

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>


namespace Ui {
class GameWidget;
}

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

    void initBoard();
    void initUI();
    void initConnect();
    void drawChess(int x,int y,ChessType chessType);
    void undoForUI();
    void clearBoardForUI();

    bool checkPoint(int x,int y);

    QPoint posToGrid(const QPoint &pos);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void updateViewScale();
public:

    GamemodeType getCurrentGamemode() const;
    void setCurrentGamemode(GamemodeType newCurrentGamemode);

private:
    Ui::GameWidget *ui;

    QGraphicsView* view;
    QGraphicsScene* scene;
    QGraphicsPixmapItem* boardItem;

    GameSession* session;

    GamemodeType currentGamemode;
    int currentStep = 0;

    BoardDataPtr boardData;

    // int board[15][15]; // 0空1黑2白
    int currentColor = 1;
    int nextColor = 2;
    QList<QGraphicsPixmapItem*> chessItems;
    QList<QPoint> chessPoints;

    const qreal BORDER_RATIO = 22.5/535;

    const QString PATH_blackChess = ":/pic/res/picture/black_chess.png";
    const QString PATH_whiteChess = ":/pic/res/picture/white_chess.png";
    const QString PATH_board = ":/pic/res/picture/game_background.bmp";
signals:
    void signal_changeGamemode(GamemodeType gamemode);
    void signal_mouseClicked(int x,int y);
    void signal_resetBoard();
    void signal_undoRequest();
    void signal_yourTurn();
public slots:
    void slot_changeGamemode(GamemodeType gamemode);
    void slot_undo();
    void slot_reset();
    void slot_drawChess(int x,int y,ChessType chessType);
    void slot_switchTurn();
    void slot_playerWin(AbstractPlayer* player);
};

#endif // GAMEWIDGET_H
