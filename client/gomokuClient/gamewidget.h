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
#include <QPointer>
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
    void setCurrentGamemode(GamemodeType newCurrentGamemode);

    void updatePlayerInfoUI();    // 初始化/更新玩家基础信息
    void updateTimeForUi(int s); // 更新思考时间,仅ui
    void updateTimeForUi(QString currentPlayer,int s); // 在线模式用
    void updateActiveHighlight(); // 更新当前落子方的边框高亮
    void updateActiveHighlight(QString currentPlayer); // 在线模式用
    void updateThinkingStaticUi(QString currentPlayer);
    void createOverlayWidgets(); // 初始化在线模式悔棋流程非阻塞 UI
    void updateLastChessMarker(qreal chessX, qreal chessY, int chessW, int chessH);

    bool checkPoint(int x,int y);
    bool isAIMode();

    QString formatTime(int s);    // 格式化时间显示

    QPoint posToGrid(const QPoint &pos);

    GamemodeType getCurrentGamemode() const;


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
    QPointer<QProgressDialog> undoWaitDialog;    // 悔棋等待遮罩
    QPointer<QWidget> undoConfirmPanel;   // 悔棋确认面板
    QPointer<QDialog> m_gameOverDialog = nullptr;
    bool m_isReplayNegotiating = false;
    GameSession* session;

    QTimer *m_turnTimer = nullptr;          // 回合计时器
    int m_elapsedSeconds = 0;         // 当前回合已用秒数

    GamemodeType currentGamemode;
    int currentStep = 0;

    BoardDataPtr boardData;

    int currentColor = 1;
    int nextColor = 2;
    QList<QGraphicsPixmapItem*> chessItems; // 仅ui棋子
    QList<QPoint> chessPoints;

    const qreal BORDER_RATIO = 22.5/535;

    const QString PATH_blackChess = ":/pic/res/picture/black_chess.png";
    const QString PATH_whiteChess = ":/pic/res/picture/white_chess.png";
    const QString PATH_board = ":/pic/res/picture/game_background.bmp";

    QPointer<QLabel> aiThinkingLabel; // “AI 正在思考”文本
    QPointer<QLabel> aiLoadingGif;    // 加载动画
    QPointer<QMovie> loadingMovie;    // GIF 动画资源
    QGraphicsRectItem* m_lastChessMarker; // 四角高亮标记
    QPoint m_lastChessPos;               // 最后落子坐标
    const int CHESS_MARKER_PADDING = 3;  // 标记内边距
signals:
    void signal_changeGamemode(GamemodeType gamemode);
    void signal_mouseClicked(int x,int y);
    void signal_resetBoard();
    void signal_undoRequestOffline();
    void signal_yourTurn();
    void signal_setOnlineTimerState(bool state);
public slots:
    void slot_changeGamemode(GamemodeType gamemode);
    void slot_undoOffline();
    void slot_reset();
    void slot_drawChess(int x,int y,ChessType chessType);
    void slot_drawChessForOnline(int x,int y,int color,bool status);
    void slot_switchTurn();
    void slot_onTimerTimeout();
    void slot_playerWin(AbstractPlayer* player);
    void slot_onUpdateThinkingTime(int s);

    void slot_onlineGameOver(QString msg);
    void slot_onGameOverDisconnectReceived(QString roomId,QString msg);
    // 再来一局相关槽函数
    void slot_onReplayChoiceReceived(QString roomId, QString player, bool status, QString msg);
    void slot_onReplayStartReceived(QString roomId, QString newColor, QString msg);
    void slot_onReplayCancelReceived(QString roomId, QString player,QString msg);
};

#endif // GAMEWIDGET_H
