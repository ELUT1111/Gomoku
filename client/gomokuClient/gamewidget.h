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
#include <QLabel>
#include <qprogressdialog.h>

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
    bool isAIMode();

    QPoint posToGrid(const QPoint &pos);

    void createOverlayWidgets(); // 初始化在线模式悔棋流程非阻塞 UI

    GamemodeType getCurrentGamemode() const;
    void setCurrentGamemode(GamemodeType newCurrentGamemode);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void updateViewScale();

private:
    Ui::GameWidget *ui;

    QGraphicsView* view;
    QGraphicsScene* scene;
    QGraphicsPixmapItem* boardItem;
    QProgressDialog* undoWaitDialog;    // 悔棋等待遮罩
    QWidget* undoConfirmPanel;   // 悔棋确认面板
    QDialog* m_gameOverDialog = nullptr;
    bool m_isReplayNegotiating = false;
    GameSession* session;

    GamemodeType currentGamemode;
    int currentStep = 0;

    BoardDataPtr boardData;

    // int board[15][15]; // 0空1黑2白
    int currentColor = 1;
    int nextColor = 2;
    QList<QGraphicsPixmapItem*> chessItems; // 仅ui棋子
    QList<QPoint> chessPoints;

    const qreal BORDER_RATIO = 22.5/535;

    const QString PATH_blackChess = ":/pic/res/picture/black_chess.png";
    const QString PATH_whiteChess = ":/pic/res/picture/white_chess.png";
    const QString PATH_board = ":/pic/res/picture/game_background.bmp";

    QLabel *aiThinkingLabel; // “AI 正在思考”文本
    QLabel *aiLoadingGif;    // 加载动画
    QMovie *loadingMovie;    // GIF 动画资源
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
    void slot_drawChessForOnline(int x,int y,int color,bool status);
    void slot_switchTurn();
    void slot_playerWin(AbstractPlayer* player);

    void slot_onlineGameOver(QString msg);
    void slot_onGameOverDisconnectReceived(QString roomId,QString msg);
    // 再来一局相关槽函数
    void slot_onReplayChoiceReceived(QString roomId, QString player, bool status, QString msg);
    void slot_onReplayStartReceived(QString roomId, QString newColor, QString msg);
    void slot_onReplayCancelReceived(QString roomId, QString player,QString msg);
};

#endif // GAMEWIDGET_H
