#include "gamewidget.h"
#include "ui_gamewidget.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMessageBox>

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

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    // 场景+视图
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::SmoothPixmapTransform); // 平滑缩放
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setStyleSheet("background-color:green");
    layout->addWidget(view);

    // 加载棋盘图片
    QPixmap boardPix(PATH_board);
    boardItem = new QGraphicsPixmapItem(boardPix);
    scene->addItem(boardItem);

    // 设置场景大小
    scene->setSceneRect(boardItem->boundingRect());
}

void GameWidget::initConnect()
{
    connect(this,&GameWidget::signal_mouseClicked,GameSession::instance()->currentPlayer,&AbstractPlayer::slot_onMouseClicked);
    connect(GameSession::instance(),&GameSession::signal_drawChess,this,&GameWidget::slot_drawChess);
    connect(GameSession::instance(),&GameSession::signal_switchTurn,this,&GameWidget::slot_switchTurn);
}

void GameWidget::drawChess(int x, int y, ChessType chessType)
{
    if(boardData->getChess(x,y) != ChessType::EMPTY) return;
    //boardData->setChess(x,y,chessType);

    // TODO

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
}

void GameWidget::undoForUI()
{
    /*悔棋*/
    if(chessItems.isEmpty()) return;
    auto last = chessItems.takeLast();
    scene->removeItem(last);
    delete last;
    last = nullptr;
    //QPoint lastGird = chessPoints.takeLast();
    //board[lastGird.x()][lastGird.y()] = 0;
    std::swap(currentColor,nextColor);
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
}

bool GameWidget::checkPoint(int x, int y)
{
    return x>=0 && x<15 && y>=0 && y<15;
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

    // 映射到15×15网格交点（0~14）
    int x = qRound(relative_x / grid_w * (BOARD_SIZE - 1));
    int y = qRound(relative_y / grid_h * (BOARD_SIZE - 1));

    // 最终确保x/y在0~14范围内
    x = qBound(0, x, BOARD_SIZE - 1);
    y = qBound(0, y, BOARD_SIZE - 1);

    return QPoint(x, y);
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
}

void GameWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->button() == Qt::LeftButton)
    {
        QPoint grid = posToGrid(event->pos());
        int x = grid.x();
        int y = grid.y();
        if( boardData->getChess(x, y) == ChessType::EMPTY) // 空位
        {
            qDebug()<<grid;
            emit signal_mouseClicked(x,y);
            // placeChess(x, y, currentColor);
            // chessPoints.append(grid);
            // std::swap(currentColor,nextColor);
            // if(checkWin(x,y,nextColor))
            // {
            //     QString winner = nextColor==1? "黑方":"白方";
            //     QMessageBox::information(this,"游戏结束",winner+"获胜!");
            //     clearBoard();
            // }
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

GamemodeType GameWidget::getCurrentGamemode() const
{
    return currentGamemode;
}

void GameWidget::setCurrentGamemode(GamemodeType newCurrentGamemode)
{
    currentGamemode = newCurrentGamemode;
}

void GameWidget::slot_undo()
{
    undoForUI();
}

void GameWidget::slot_reset()
{
    clearBoardForUI();
}

void GameWidget::slot_drawChess(int x, int y, ChessType chessType)
{
    drawChess(x,y,chessType);
}

void GameWidget::slot_switchTurn()
{
    //qDebug()<<"交换";
    disconnect(this,&GameWidget::signal_mouseClicked,GameSession::instance()->lastPlayer,&AbstractPlayer::slot_onMouseClicked);
    connect(this,&GameWidget::signal_mouseClicked,GameSession::instance()->currentPlayer,&AbstractPlayer::slot_onMouseClicked);
}
