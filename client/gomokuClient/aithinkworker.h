#ifndef AITHINKWORKER_H
#define AITHINKWORKER_H

#include <QObject>
#include <QPoint>
#include "SharedType.h"

class AIThinkWorker : public QObject
{
    Q_OBJECT
public:
    explicit AIThinkWorker(QObject *parent = nullptr,
                           ChessType chessType = ChessType::WHITE,
                           AIType difficulty = AIType::EASY);
    void setDifficulty(AIType difficulty);
    void setChessType(ChessType chessType);

public slots:
    void doThink(); // 执行思考的核心槽函数

signals:
    void thinkFinished(QPoint pos); // 思考完成，返回落子位置
    void thinkStarted();            // 思考开始信号

private:
    // 思考逻辑
    QPoint nextPos();
    QPoint nextPosForEasy();
    QPoint nextPosForNormal();
    QPoint nextPosForHard();
    QPoint randomEmptyPos();
    QList<QPoint> generateCandidateMoves() const;
    int evaluateBoard() const;
    int minimax(int depth, int alpha, int beta, ChessType currentPlayer);
    QPoint minimaxDecision(int depth);

    ChessType m_chessType;
    AIType m_difficulty;
    static constexpr int INF = 1000000;
};

#endif // AITHINKWORKER_H
