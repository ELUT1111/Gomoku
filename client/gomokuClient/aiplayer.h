#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "abstractplayer.h"
#include <QPoint>
#include <QList>
#include <QThread>
#include <AIThinkWorker.h>

class AIPlayer : public AbstractPlayer
{
    Q_OBJECT
public:
    explicit AIPlayer(QObject *parent = nullptr,ChessType chessType = ChessType::BLACK, AIType difficutify = AIType::EASY);
    ~AIPlayer() override;
    AIType getDifficulty() const;
    void setDifficulty(AIType newDifficulty);

    void startThinking_func();
    void cancelThinking();

protected:
    AIType difficulty;
    int isThinking = 0;

    // 线程核心成员
    QThread *aiThread;
    AIThinkWorker *thinkWorker;

public slots:
    void slot_onMouseClicked(int x,int y) override;
    void slot_myTurn() override;
    void onThinkFinished(QPoint pos); // 接收 worker 的思考结果
signals:
    void startThinking(); // 触发 AI 思考的信号（给 worker）
    void thinkStarted();  // AI 开始思考（给 UI）
    void thinkFinished(); // AI 思考完成（给 UI）
};

#endif // AIPLAYER_H
