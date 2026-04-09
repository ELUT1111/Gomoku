#include "AIPlayer.h"
#include "PageManager.h"
#include "gamewidget.h"
#include "qtimer.h"
#include "ui_gamewidget.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMessageBox>
#include <HumanPlayer.h>
#include <QMovie>
#include <OnlineSessionManager.h>
#include <QPushButton>


GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , session(nullptr)
{
    ui->setupUi(this);

    initBoard();

    initUI();

    initConnect();

}

GameWidget::~GameWidget()
{
    delete ui;
}

void GameWidget::initBoard()
{
    boardData = GameSession::instance()->getBoardData();
}

void GameWidget::initUI()
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 随父容器扩展
    this->setContentsMargins(0, 0, 0, 0); // 移除自身边距

    // 场景+视图
    scene = new QGraphicsScene(this);
    view = ui->graphicsView_board;
    view->setScene(scene);
    view->setRenderHint(QPainter::SmoothPixmapTransform); // 平滑缩放
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setStyleSheet("background-color:green");
    // layout->addWidget(view);

    // 加载棋盘图片
    QPixmap boardPix(PATH_board);
    boardItem = new QGraphicsPixmapItem(boardPix);
    scene->addItem(boardItem);

    // 加载动画
    aiLoadingGif = new QLabel(this);
    loadingMovie = new QMovie(":/pic/res/picture/loading.gif");
    aiLoadingGif->setMovie(loadingMovie);
    aiLoadingGif->setFixedSize(100, 100); // 动画大小
    aiLoadingGif->setAlignment(Qt::AlignCenter);
    aiLoadingGif->setStyleSheet("background: transparent;");
    aiLoadingGif->hide();

    // 文本提示
    aiThinkingLabel = new QLabel("AI 正在思考...", this);
    aiThinkingLabel->setStyleSheet("color: white; font-size: 16px; background: rgba(0,0,0,0.5); padding: 8px; border-radius: 8px;");
    aiThinkingLabel->setAlignment(Qt::AlignCenter);
    aiThinkingLabel->hide();

    // 叠加到棋盘视图上
    aiLoadingGif->setParent(view);
    aiThinkingLabel->setParent(view);

    m_lastChessMarker = new QGraphicsRectItem();
    m_lastChessMarker->setPen(QPen(QColor("#FFD700"), 3, Qt::SolidLine)); // 金色粗边框
    m_lastChessMarker->setBrush(Qt::NoBrush); // 无填充
    m_lastChessMarker->setZValue(2); // 置于棋子上层
    m_lastChessMarker->hide(); // 初始隐藏
    scene->addItem(m_lastChessMarker);
    m_lastChessPos = QPoint(-1, -1);

    // 设置场景大小
    scene->setSceneRect(boardItem->boundingRect());

    createOverlayWidgets(); // 初始化在线模式悔棋流程ui

    updateActiveHighlight("BLACK"); // 初始化思考状态ui

    // 初始化计时器
    m_turnTimer = new QTimer(this);
    m_turnTimer->setInterval(1000); // 1秒触发一次
    connect(m_turnTimer, &QTimer::timeout, this, &GameWidget::slot_onTimerTimeout);

    m_elapsedSeconds = 0;
}

void GameWidget::initConnect()
{
    // qDebug()<<"[gameWidget] 初始化连接";
    connect(PageManager::instance(),&PageManager::signal_changeGamemode,this,&GameWidget::slot_changeGamemode,Qt::UniqueConnection);


    // connect(this,&GameWidget::signal_mouseClicked,GameSession::instance()->currentPlayer,&AbstractPlayer::slot_onMouseClicked);
    connect(GameSession::instance(),&GameSession::signal_drawChess,this,&GameWidget::slot_drawChess,Qt::UniqueConnection);
    connect(GameSession::instance(),&GameSession::signal_switchTurn,this,&GameWidget::slot_switchTurn,Qt::UniqueConnection);
    connect(GameSession::instance(),&GameSession::signal_playerWin,this,&GameWidget::slot_playerWin,Qt::UniqueConnection);

    connect(this,&GameWidget::signal_resetBoard,GameSession::instance(),&GameSession::slot_resetGame,Qt::UniqueConnection);
    connect(this,&GameWidget::signal_undoRequest,GameSession::instance(),&GameSession::slot_handleUndo,Qt::UniqueConnection);
    connect(this,&GameWidget::signal_changeGamemode,GameSession::instance(),&GameSession::slot_changeGamemode,Qt::UniqueConnection);

    // 绑定在线游戏落子状态返回信号
    connect(&NetworkManager::instance(),&NetworkManager::sig_placeChessStatusReceived,this,&GameWidget::slot_drawChessForOnline);
    // 绑定在线游戏结束信号
    connect(GameSession::instance(), &GameSession::sig_onlineGameOver, this, &GameWidget::slot_onlineGameOver);
    // 绑定收到悔棋请求信号
    connect(&NetworkManager::instance(),&NetworkManager::sig_undoRequestReceived,this,[this](QString roomId,QString player,bool status){

        if(!this->isVisible()) return;

        if(status)
        {
            if(OnlineSessionManager::getMyOnlineColor() == player)
            {
                // 创建等候弹窗
                undoWaitDialog = new QProgressDialog("正在等待对方回复...", "取消", 0, 0, this);
                undoWaitDialog->setWindowTitle("等待悔棋中");
                undoWaitDialog->setWindowModality(Qt::WindowModal);
                undoWaitDialog->setMinimumDuration(0); // 立即显示

                // 点击取消发送取消请求
                connect(undoWaitDialog, &QProgressDialog::canceled, this, [](){
                    NetworkManager::instance().sendUndoRequest(false);
                });

                undoWaitDialog->show();
            }
            else
            {
                undoConfirmPanel->move((width()-280)/2, (height()-120)/2);
                undoConfirmPanel->show();
                undoConfirmPanel->raise();
            }
        }
        else //收到取消悔棋请求
        {
            if(OnlineSessionManager::getMyOnlineColor() == player)
            {
                if(undoWaitDialog) {
                    undoWaitDialog->deleteLater();
                    undoWaitDialog = nullptr;
                }
            }
            else
            {
                undoConfirmPanel->hide();
            }
        }

    });
    // 绑定悔棋请求返回状态信号
    connect(&NetworkManager::instance(),&NetworkManager::sig_undoStatusReceived,this,[this](QString roomId,QString player,bool status){
        if(!this -> isVisible()) return;
        if(undoWaitDialog) {
            undoWaitDialog->deleteLater();
            undoWaitDialog = nullptr;
        }
        if(status)
        {
            undoForUI();
            OnlineSessionManager::instance()->switchCurrentThinkingPlayer();
            emit signal_setOnlineTimerState(true);
            updateThinkingStaticUi(OnlineSessionManager::instance()->getCurrentThinkingPlayer());
        }
    });

    // 绑定在线错误信号
    connect(GameSession::instance(), &GameSession::sig_onlineError, this, [this](QString msg){
        QMessageBox::warning(this, "在线错误", msg);
    });
    // 绑定再来一局相关信号
    connect(&NetworkManager::instance(), &NetworkManager::sig_replayChoiceReceived,
            this, &GameWidget::slot_onReplayChoiceReceived);
    connect(&NetworkManager::instance(), &NetworkManager::sig_replayStartReceived,
            this, &GameWidget::slot_onReplayStartReceived);
    connect(&NetworkManager::instance(), &NetworkManager::sig_replayCancelReceived,
            this, &GameWidget::slot_onReplayCancelReceived);
    connect(&NetworkManager::instance(), &NetworkManager::sig_gameOverDisconnectReceived,
            this, &GameWidget::slot_onGameOverDisconnectReceived);
    // 监听 AI 思考状态
    connect(GameSession::instance(), &GameSession::signal_switchTurn, this, [this]() {
        if(GameSession::instance()->gamemode == GamemodeType::ONLINE) return;

        AIPlayer *ai = qobject_cast<AIPlayer*>(GameSession::instance()->currentPlayer);
        if (ai) {
            // 连接 AI 思考信号
            disconnect(ai, &AIPlayer::thinkStarted, this, nullptr);
            disconnect(ai, &AIPlayer::thinkFinished, this, nullptr);

            connect(ai, &AIPlayer::thinkStarted, this, [this]() {
                // AI 开始思考：显示动画 + 禁用棋盘点击
                aiLoadingGif->show();
                aiThinkingLabel->show();
                loadingMovie->start();
                view->setEnabled(false); // 禁用人类操作
            });

            connect(ai, &AIPlayer::thinkFinished, this, [this]() {
                // AI 思考完成：隐藏动画 + 启用棋盘点击
                aiLoadingGif->hide();
                aiThinkingLabel->hide();
                loadingMovie->stop();
                view->setEnabled(true);
            });
        }
    });

    connect(this,&GameWidget::signal_setOnlineTimerState,OnlineSessionManager::instance(),
            &OnlineSessionManager::slot_setTimerStatus);
    connect(OnlineSessionManager::instance(),&OnlineSessionManager::signal_updateThinkingTime,
            this,&GameWidget::slot_onUpdateThinkingTime);

}

void GameWidget::drawChess(int x, int y, ChessType chessType)
{
    if(GameSession::instance()->gamemode != GamemodeType::ONLINE && boardData->getChess(x,y) != ChessType::EMPTY) return;
    qDebug()<<"[gameWidget] 绘子"<<x<<","<<y;

    // 加载棋子图片
    QPixmap chess_pix = (chessType == ChessType::BLACK) ? QPixmap(PATH_blackChess) : QPixmap(PATH_whiteChess);
    QGraphicsPixmapItem *chess_item = new QGraphicsPixmapItem(chess_pix);
    scene->addItem(chess_item);

    // 计算有效网格区域的尺寸和偏移
    QRectF board_rect = boardItem->boundingRect();
    qreal board_w = board_rect.width();
    qreal board_h = board_rect.height();
    qreal grid_left = board_w * BORDER_RATIO;
    qreal grid_top = board_h * BORDER_RATIO;
    qreal grid_w = board_w * (1 - 2 * BORDER_RATIO);
    qreal grid_h = board_h * (1 - 2 * BORDER_RATIO);

    // 计算第(x,y)个网格交点的场景坐标
    qreal grid_step_x = grid_w / (BOARD_SIZE - 1); // 每个网格的宽度
    qreal grid_step_y = grid_h / (BOARD_SIZE - 1); // 每个网格的高度
    qreal chess_scene_x = grid_left + x * grid_step_x;
    qreal chess_scene_y = grid_top + y * grid_step_y;

    // 让棋子图片居中到网格交点
    chess_item->setPos(
        chess_scene_x - chess_pix.width() / 2,
        chess_scene_y - chess_pix.height() / 2
        );

    chessItems.append(chess_item);
    chessPoints.append(QPoint(x, y)); // 记录落子坐标

    // 新棋子标记
    m_lastChessPos = QPoint(x, y);
    updateLastChessMarker(chess_scene_x - chess_pix.width()/2,
                          chess_scene_y - chess_pix.height()/2,
                          chess_pix.width(),
                          chess_pix.height());
}

void GameWidget::undoForUI()
{
    /*悔棋*/

    if(chessItems.isEmpty())
    {
        m_lastChessMarker->hide();
        m_lastChessPos = QPoint(-1, -1);
        chessPoints.clear();
        return;
    }
    auto last = chessItems.takeLast();
    chessPoints.takeLast();
    scene->removeItem(last);
    delete last;
    last = nullptr;

    if(isAIMode())
    {
        // 离线ai模式下连悔两棋
        if(chessItems.isEmpty()) return;
        auto last = chessItems.takeLast();
        chessPoints.takeLast();
        scene->removeItem(last);
        delete last;
        last = nullptr;
    }

    // 悔棋后更新标记
    if(!chessPoints.isEmpty())
    {
        QPoint lastPos = chessPoints.last();
        QRectF board_rect = boardItem->boundingRect();
        qreal board_w = board_rect.width();
        qreal board_h = board_rect.height();
        qreal grid_left = board_w * BORDER_RATIO;
        qreal grid_top = board_h * BORDER_RATIO;
        qreal grid_w = board_w * (1 - 2 * BORDER_RATIO);
        qreal grid_h = board_h * (1 - 2 * BORDER_RATIO);
        qreal grid_step_x = grid_w / (BOARD_SIZE - 1);
        qreal grid_step_y = grid_h / (BOARD_SIZE - 1);
        qreal cx = grid_left + lastPos.x() * grid_step_x;
        qreal cy = grid_top + lastPos.y() * grid_step_y;
        QPixmap pix = (boardData->getChess(lastPos.x(), lastPos.y())==ChessType::BLACK)
                          ? QPixmap(PATH_blackChess) : QPixmap(PATH_whiteChess);
        updateLastChessMarker(cx-pix.width()/2, cy-pix.height()/2, pix.width(), pix.height());
        m_lastChessPos = lastPos;
    }
    else
    {
        m_lastChessMarker->hide();
        m_lastChessPos = QPoint(-1, -1);
    }
}

void GameWidget::clearBoardForUI()
{

    currentColor = 1;
    nextColor = 2;
    //initBoard();
    for(auto chess:chessItems)
    {
        scene->removeItem(chess);
        delete chess;
    }
    chessItems.clear();
    chessPoints.clear();

    if(m_lastChessMarker)
    {
        m_lastChessMarker->hide();
        m_lastChessPos = QPoint(-1, -1);
    }
}

bool GameWidget::checkPoint(int x, int y)
{
    return x>=0 && x<15 && y>=0 && y<15;
}

bool GameWidget::isAIMode()
{
    return (currentGamemode == GamemodeType::OFFLINE_AI_EASY ||
            currentGamemode == GamemodeType::OFFLINE_AI_NORMAL ||
            currentGamemode == GamemodeType::OFFLINE_AI_HARD);
}

QPoint GameWidget::posToGrid(const QPoint &pos)
{
    // 鼠标坐标 → View坐标 → 场景坐标
    QPointF scene_pos = view->mapToScene(pos);

    // 整张棋盘图片的尺寸
    QRectF board_rect = boardItem->boundingRect();
    qreal board_w = board_rect.width();
    qreal board_h = board_rect.height();

    // 计算有效网格区域的偏移和尺寸
    qreal grid_left = board_w * BORDER_RATIO;
    qreal grid_top = board_h * BORDER_RATIO;
    qreal grid_w = board_w * (1 - 2 * BORDER_RATIO);
    qreal grid_h = board_h * (1 - 2 * BORDER_RATIO);

    // 把场景坐标转换为有效网格区域内的相对坐标
    qreal relative_x = scene_pos.x() - grid_left;
    qreal relative_y = scene_pos.y() - grid_top;

    // 限制坐标在有效网格内
    relative_x = qBound(0.0, relative_x, grid_w);
    relative_y = qBound(0.0, relative_y, grid_h);

    // 映射到网格交点
    int x = qRound(relative_x / grid_w * (BOARD_SIZE - 1));
    int y = qRound(relative_y / grid_h * (BOARD_SIZE - 1));

    // 最终确保x/y在0~14范围内
    x = qBound(0, x, BOARD_SIZE - 1);
    y = qBound(0, y, BOARD_SIZE - 1);

    return QPoint(x, y);
}

void GameWidget::createOverlayWidgets()
{
    undoConfirmPanel = new QWidget(this);
    undoConfirmPanel->setObjectName("undoPanel");
    undoConfirmPanel->setFixedSize(280, 120);
    undoConfirmPanel->setStyleSheet(
        "#undoPanel { background: white; border: 2px solid #4A6CF7; border-radius: 10px; }"
        "QPushButton { background: #4A6CF7; color: white; border-radius: 5px; padding: 5px; }"
        );

    QVBoxLayout* layout = new QVBoxLayout(undoConfirmPanel);
    QLabel* txt = new QLabel("对方请求悔棋...", undoConfirmPanel);
    txt->setAlignment(Qt::AlignCenter);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* btnYes = new QPushButton("同意", undoConfirmPanel);
    QPushButton* btnNo = new QPushButton("拒绝", undoConfirmPanel);
    btnLayout->addWidget(btnYes);
    btnLayout->addWidget(btnNo);

    layout->addWidget(txt);
    layout->addLayout(btnLayout);
    undoConfirmPanel->hide();

    // 悔棋决定
    connect(btnYes, &QPushButton::clicked, [this](){
        NetworkManager::instance().sendUndoChoice(true);
        undoConfirmPanel->hide();
    });
    connect(btnNo, &QPushButton::clicked, [this](){
        NetworkManager::instance().sendUndoChoice(false);
        undoConfirmPanel->hide();
    });
}

void GameWidget::updateLastChessMarker(qreal chessX, qreal chessY, int chessW, int chessH)
{
    if (!m_lastChessMarker) return;
    // 计算标记矩形
    qreal markerX = chessX - CHESS_MARKER_PADDING;
    qreal markerY = chessY - CHESS_MARKER_PADDING;
    qreal markerW = chessW + 2 * CHESS_MARKER_PADDING;
    qreal markerH = chessH + 2 * CHESS_MARKER_PADDING;
    m_lastChessMarker->setRect(markerX, markerY, markerW, markerH);
    m_lastChessMarker->show();
}

void GameWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateViewScale();
}

void GameWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateViewScale();

    // 调整加载控件位置
    if (aiLoadingGif && aiThinkingLabel) {
        QSize gifSize = aiLoadingGif->size();
        aiLoadingGif->move(view->width()/2 - gifSize.width()/2,
                           view->height()/2 - gifSize.height()/2 - 30);
        aiThinkingLabel->move(view->width()/2 - aiThinkingLabel->width()/2,
                              view->height()/2 + gifSize.height()/2);
    }
}

void GameWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    // 悔棋请求中禁用
    if(undoWaitDialog != nullptr && undoWaitDialog->isVisible()) return;
    if(event->button() == Qt::LeftButton)
    {
        QPoint viewPos = ui->graphicsView_board->mapFrom(this, event->pos());
        if(ui->graphicsView_board->viewport()->rect().contains(viewPos))
        {
            QPoint grid = posToGrid(viewPos);
            int x = grid.x();
            int y = grid.y();

            if(GameSession::instance()->gamemode == GamemodeType::ONLINE)
            {
                //ChessType myType = (g_myOnlineTag == "BLACK") ? ChessType::BLACK : ChessType::WHITE;

                NetworkManager::instance().sendChessMove(x,y,OnlineSessionManager::instance()->getMyOnlineColor());

                return;
            }
            if( boardData->getChess(x, y) == ChessType::EMPTY) // 空位
            {
                qDebug()<<grid;
                emit signal_mouseClicked(x,y);
            }
        }
    }
}

void GameWidget::updateViewScale()
{
    if(!boardItem) return;

    // 视图大小
    QRectF viewRect = view->viewport()->rect();
    // 棋盘原图大小
    QRectF boardRect = boardItem->boundingRect();

    // 计算等比缩放比例
    qreal scaleW = viewRect.width() / boardRect.width();
    qreal scaleH = viewRect.height() / boardRect.height();
    qreal scale = qMin(scaleW, scaleH);

    // 重置变换
    view->resetTransform();
    view->scale(scale, scale);

    // 居中
    view->centerOn(boardItem);
}

QString GameWidget::formatTime(int s)
{
    if (s < 0) {
        s = 0;
    }

    const int hours = s / 3600;
    const int remainingSeconds = s % 3600;
    const int minutes = remainingSeconds / 60;
    const int seconds = remainingSeconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours).arg(minutes).arg(seconds);
    }
    else {
        return QString("%1:%2").arg(minutes).arg(seconds);
    }
}

GamemodeType GameWidget::getCurrentGamemode() const
{
    return currentGamemode;
}

void GameWidget::setCurrentGamemode(GamemodeType newCurrentGamemode)
{
    qDebug()<<"[gameWidget] 设置模式:"<<int(newCurrentGamemode);
    currentGamemode = newCurrentGamemode;
    emit signal_changeGamemode(newCurrentGamemode);
}

void GameWidget::updatePlayerInfoUI()
{

}

void GameWidget::updateTimeForUi(int s)
{
    QString timeStr = formatTime(s);

    if (GameSession::instance()->currentPlayer->getMyChessType() == ChessType::BLACK) {
        ui->lbl_time_black->setText(timeStr);
    } else {
        ui->lbl_time_white->setText(timeStr);
    }
}

void GameWidget::updateTimeForUi(QString currentPlayer, int s)
{
    QString timeStr = formatTime(s);

    if (currentPlayer == "BLACK") {
        ui->lbl_time_black->setText(timeStr);
    } else {
        ui->lbl_time_white->setText(timeStr);
    }
}

void GameWidget::updateActiveHighlight()
{
    QString activeStyle = "QFrame { border: 3px solid #2563EB; border-radius: 10px; background-color: rgba(74, 108, 247, 0.05); }";
    QString inactiveStyle = "QFrame { border: 3px solid #EEEEEE; border-radius: 10px; background-color: transparent; }";

    if (GameSession::instance()->currentPlayer->getMyChessType() == ChessType::BLACK) {
        ui->frame_black->setStyleSheet(activeStyle);
        ui->frame_white->setStyleSheet(inactiveStyle);
    } else {
        ui->frame_black->setStyleSheet(inactiveStyle);
        ui->frame_white->setStyleSheet(activeStyle);
    }
}

void GameWidget::updateActiveHighlight(QString currentPlayer)
{
    QString activeStyle = "QFrame { border: 3px solid #2563EB; border-radius: 10px; background-color: rgba(74, 108, 247, 0.05); }";
    QString inactiveStyle = "QFrame { border: 3px solid #EEEEEE; border-radius: 10px; background-color: transparent; }";

    if (currentPlayer == "BLACK") {
        ui->frame_black->setStyleSheet(activeStyle);
        ui->frame_white->setStyleSheet(inactiveStyle);
    } else {
        ui->frame_black->setStyleSheet(inactiveStyle);
        ui->frame_white->setStyleSheet(activeStyle);
    }
}

void GameWidget::updateThinkingStaticUi(QString currentPlayer)
{
    if(currentPlayer == "BLACK")
    {
        ui->lbl_time_black->setText("0:0");
        ui->lbl_time_white->setText("");
        ui->lbl_status_black->setText("思考中");
        ui->lbl_status_white->setText("等待中");
    }
    else
    {
        ui->lbl_time_black->setText("");
        ui->lbl_time_white->setText("0:0");
        ui->lbl_status_black->setText("等待中");
        ui->lbl_status_white->setText("思考中");
    }
    updateActiveHighlight(currentPlayer);
}

void GameWidget::slot_changeGamemode(GamemodeType gamemode)
{
    setCurrentGamemode(gamemode);
}

void GameWidget::slot_undo()
{
    if(currentGamemode != GamemodeType::ONLINE)emit signal_undoRequest();
    undoForUI();
}

void GameWidget::slot_reset()
{
    emit signal_resetBoard();
    clearBoardForUI();
}

void GameWidget::slot_drawChess(int x, int y, ChessType chessType)
{
    drawChess(x,y,chessType);
}

void GameWidget::slot_drawChessForOnline(int x, int y, int color, bool status)
{
    if(status)
    {
        drawChess(x , y, (ChessType)color);
        OnlineSessionManager::instance()->switchCurrentThinkingPlayer();
        emit signal_setOnlineTimerState(true);
        updateThinkingStaticUi(OnlineSessionManager::instance()->getCurrentThinkingPlayer());
    }
}

void GameWidget::slot_switchTurn()
{
    if(currentGamemode == GamemodeType::ONLINE) return;

    m_elapsedSeconds = 0;
    m_turnTimer->start();

    ui->lbl_time_black->setText(GameSession::instance()->currentPlayer->getMyChessType() == ChessType::BLACK ? "0:0" : "");
    ui->lbl_time_white->setText(GameSession::instance()->currentPlayer->getMyChessType() == ChessType::WHITE ? "0:0" : "");
    ui->lbl_status_black->setText(GameSession::instance()->currentPlayer->getMyChessType() == ChessType::BLACK ? "思考中" : "等待中");
    ui->lbl_status_white->setText(GameSession::instance()->currentPlayer->getMyChessType() == ChessType::WHITE ? "思考中" : "等待中");

    updateActiveHighlight();

    //qDebug()<<"[gameWidget] 交换回合";
    disconnect(this, &GameWidget::signal_mouseClicked, nullptr, nullptr);
    if (qobject_cast<HumanPlayer*>(GameSession::instance()->currentPlayer) ||
        qobject_cast<OnlinePlayer*>(GameSession::instance()->currentPlayer)) {
        connect(this,&GameWidget::signal_mouseClicked,
                GameSession::instance()->currentPlayer,&AbstractPlayer::slot_onMouseClicked,
                Qt::UniqueConnection);
    }

    if(isAIMode())
    {
        AIPlayer * ai = qobject_cast<AIPlayer*> (GameSession::instance()->currentPlayer);
        if(ai)
        {
            disconnect(this, &GameWidget::signal_yourTurn, nullptr, nullptr);
            // disconnect(this,&GameWidget::signal_yourTurn,ai,&AIPlayer::slot_myTurn);
            connect(this,&GameWidget::signal_yourTurn,ai,&AIPlayer::slot_myTurn,Qt::UniqueConnection);
            emit signal_yourTurn();
        }
    }

}

void GameWidget::slot_onTimerTimeout()
{
    m_elapsedSeconds++;
    if(GameSession::instance()->gamemode != GamemodeType::ONLINE)
    {
    updateTimeForUi(m_elapsedSeconds);
    }
}

void GameWidget::slot_playerWin(AbstractPlayer *player)
{
    m_turnTimer->stop(); // 停止计时
    QString winner = (player == GameSession::instance()->player1) ? "黑方":"白方";
    QMessageBox::information(this,"游戏结束",winner+"获胜!");
    slot_reset();
}

void GameWidget::slot_onUpdateThinkingTime(int s)
{
    if(GameSession::instance()->gamemode == GamemodeType::ONLINE)
    {
        updateTimeForUi(OnlineSessionManager::instance()->getCurrentThinkingPlayer(),s);
    }
}

void GameWidget::slot_onlineGameOver(QString msg)
{
    if(m_isReplayNegotiating) return;
    m_isReplayNegotiating = true;

    OnlineSessionManager::instance()->setCurrentThinkingPlayer("BLACK");
    emit signal_setOnlineTimerState(false);
    updateActiveHighlight("BLACK");

    // 创建自定义游戏结束弹窗
    m_gameOverDialog = new QDialog(this);
    m_gameOverDialog->setWindowTitle("游戏结束");
    m_gameOverDialog->setFixedSize(320, 200);
    m_gameOverDialog->setWindowModality(Qt::WindowModal);
    m_gameOverDialog->setStyleSheet(
        "QDialog { background-color: #F5F7FA; border-radius: 12px; }"
        "QLabel { font-size: 18px; font-weight: bold; color: #4A6CF7; }"
        "QPushButton { border-radius: 8px; border: none; font-size: 14px; height: 36px; }"
        "QPushButton#btnReplay { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4A6CF7, stop:1 #35D8A6); color: white; }"
        "QPushButton#btnQuit { background-color: white; color: #666666; border: 1px solid #EEEEEE; }"
        );

    QVBoxLayout* layout = new QVBoxLayout(m_gameOverDialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    // 结果文本
    QLabel* resultLabel = new QLabel(msg, m_gameOverDialog);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setWordWrap(true);
    layout->addWidget(resultLabel);

    // 按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    QPushButton* btnReplay = new QPushButton("再来一局", m_gameOverDialog);
    btnReplay->setObjectName("btnReplay");
    QPushButton* btnQuit = new QPushButton("退出房间", m_gameOverDialog);
    btnQuit->setObjectName("btnQuit");

    btnLayout->addWidget(btnReplay);
    btnLayout->addWidget(btnQuit);
    layout->addLayout(btnLayout);

    // 按钮点击事件
    connect(btnReplay, &QPushButton::clicked, this, [this](){
        // 发送同意再来一局
        NetworkManager::instance().sendReplayChoice(true);
        m_gameOverDialog->close();
        m_gameOverDialog->deleteLater();
        m_gameOverDialog = nullptr;
        // 显示等待提示
        // QMessageBox::information(this, "提示", "已发送再来一局请求，等待对方回复");
    });

    connect(btnQuit, &QPushButton::clicked, this, [this](){
        // 发送拒绝再来一局
        NetworkManager::instance().sendReplayChoice(false);
        m_isReplayNegotiating = false;
        m_gameOverDialog->close();
        m_gameOverDialog->deleteLater();
        m_gameOverDialog = nullptr;
        // 清空棋盘，返回在线选择页
        clearBoardForUI();
        PageManager::instance()->switchToPage(2);
    });

    m_gameOverDialog->show();
}

void GameWidget::slot_onGameOverDisconnectReceived(QString roomId, QString msg)
{
    clearBoardForUI();
    OnlineSessionManager::instance()->setCurrentThinkingPlayer("BLACK");
    OnlineSessionManager::instance()->slot_setTimerStatus(false);
    updateThinkingStaticUi("BLACK");
    QMessageBox::information(this,"游戏提示","对方已掉线");
    PageManager::instance()->switchToPage(2);
}

void GameWidget::slot_onReplayChoiceReceived(QString roomId, QString player, bool status, QString msg)
{
    clearBoardForUI();
    if(!m_isReplayNegotiating) return;
}

void GameWidget::slot_onReplayStartReceived(QString roomId, QString newColor, QString msg)
{
    m_isReplayNegotiating = false;
    // QMessageBox::information(this, "再来一局", msg);

    // 更新本地玩家颜色
    QString oldColor = OnlineSessionManager::instance()->getMyOnlineColor();
    OnlineSessionManager::instance()->setMyOnlineColor(newColor);

    //更新游戏会话中玩家的棋子类型
    ChessType oldChessType = (oldColor == "BLACK") ? ChessType::BLACK : ChessType::WHITE;
    ChessType newChessType = (newColor == "BLACK") ? ChessType::BLACK : ChessType::WHITE;
    GameSession::instance()->updateOnlinePlayerChessType(oldChessType, newChessType);

    //重置在线当前玩家标识
    OnlineSessionManager::instance()->setCurrentThinkingPlayer("BLACK");
    emit signal_setOnlineTimerState(true);
    updateThinkingStaticUi("BLACK");

    // 重置游戏与UI
    clearBoardForUI();
    GameSession::instance()->resetOnlineGameForReplay();

    // 重新绑定鼠标点击事件
    disconnect(this, &GameWidget::signal_mouseClicked, nullptr, nullptr);
    OnlinePlayer* myPlayer = GameSession::instance()->getOnlinePlayer(newChessType);
    if(myPlayer)
    {
        connect(this, &GameWidget::signal_mouseClicked,
                myPlayer, &AbstractPlayer::slot_onMouseClicked, Qt::UniqueConnection);
    }

    qDebug() << "[gameWidget] 再来一局初始化完成，新颜色：" << newColor;
}

void GameWidget::slot_onReplayCancelReceived(QString roomId, QString player, QString msg)
{
    m_isReplayNegotiating = false;

    if(m_gameOverDialog){
        m_gameOverDialog->close();
        m_gameOverDialog->deleteLater();
        m_gameOverDialog = nullptr;
    }

    OnlineSessionManager::instance()->setCurrentThinkingPlayer("BLACK");
    emit signal_setOnlineTimerState(false);
    updateThinkingStaticUi("BLACK");

    // 清空棋盘，返回在线选择页  
    clearBoardForUI();
    qDebug()<<OnlineSessionManager::getMyOnlineColor()<<"   "+player;
    if(player == OnlineSessionManager::getMyOnlineColor())
    {
        PageManager::instance()->switchToPage(2);
    }
    else
    {
        PageManager::instance()->switchToPage(3);
    }
}
