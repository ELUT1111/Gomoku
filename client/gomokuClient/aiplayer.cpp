#include "aiplayer.h"
#include "gamesession.h"
#include "qdebug.h"
#include "qpoint.h"
#include <QList>

#include <QRandomGenerator>

AIPlayer::AIPlayer(QObject *parent, ChessType chessType, AIType difficulty)
    : AbstractPlayer{parent, chessType}, difficulty(difficulty)
{
    // 1. 创建线程和 worker
    aiThread = new QThread(this);
    thinkWorker = new AIThinkWorker(nullptr, chessType, difficulty);
    thinkWorker->moveToThread(aiThread);

    // 2. 连接信号槽（线程安全）
    connect(aiThread, &QThread::finished, thinkWorker, &QObject::deleteLater);
    connect(this, &AIPlayer::startThinking, thinkWorker, &AIThinkWorker::doThink);
    connect(thinkWorker, &AIThinkWorker::thinkFinished, this, &AIPlayer::onThinkFinished);
    connect(thinkWorker, &AIThinkWorker::thinkStarted, this, &AIPlayer::thinkStarted);

    // 3. 启动线程（线程处于等待状态，仅收到信号时执行）
    aiThread->start();
}

AIPlayer::~AIPlayer()
{
    cancelThinking();
    aiThread->quit();
    aiThread->wait(); // 等待线程安全退出
}

AIType AIPlayer::getDifficulty() const
{
    return difficulty;
}

void AIPlayer::setDifficulty(AIType newDifficulty)
{
    difficulty = newDifficulty;
}

void AIPlayer::startThinking_func()
{
    if (isThinking) return;
    isThinking = true;
    emit startThinking(); // 触发 worker 执行思考
}

void AIPlayer::cancelThinking()
{
    isThinking = false;
}

void AIPlayer::slot_onMouseClicked(int x, int y)
{
    return;
}

void AIPlayer::slot_myTurn()
{
    startThinking();
}

void AIPlayer::onThinkFinished(QPoint pos)
{
    isThinking = false;
    emit thinkFinished(); // 通知 UI 思考完成
    if (pos.x() >= 0 && pos.y() >= 0) {
        emit signal_tryPlaceChess(pos.x(), pos.y(), myChessType); // 落子
    }
}
