#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

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
    void placeChess(int x,int y,int color);
    void undo();
    void clearBoard();

    int numInRow(int x,int y,int color,int dir);
    bool checkWin(int x,int y,int color);
    bool checkPoint(int x,int y);

    QPoint posToGrid(const QPoint &pos);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void updateViewScale();
public:
    int dirs[8][2] = {{0,1},{1,0},{1,1},{-1,1},{0,-1},{-1,0},{-1,-1},{1,-1}};

private:
    Ui::GameWidget *ui;

    QGraphicsView* view;
    QGraphicsScene* scene;
    QGraphicsPixmapItem* boardItem;

    int board[15][15]; // 0空1黑2白
    int currentColor = 1;
    int nextColor = 2;
    QList<QGraphicsPixmapItem*> chessItems;
    QList<QPoint> chessPoints;

    const qreal BORDER_RATIO = 22.5/535;
    const int GRID_COUNT = 15;

    const QString PATH_blackChess = ":/pic/res/picture/black_chess.png";
    const QString PATH_whiteChess = ":/pic/res/picture/white_chess.png";
    const QString PATH_board = ":/pic/res/picture/game_background.bmp";

public slots:
    void slot_undo();
    void slot_reset();
};

#endif // GAMEWIDGET_H
