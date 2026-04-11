#ifndef AITHINKWORKER_H
#define AITHINKWORKER_H

#include <QObject>
#include <QPoint>
#include <QMap>
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
    QPoint checkImmediateThreat();
    QPoint randomEmptyPos();
    QList<QPoint> generateCandidateMoves() const;
    int evaluateBoard() const;
    int minimax(int depth, int alpha, int beta, ChessType currentPlayer);
    QPoint minimaxDecision(int depth);

    int m_localBoard[BOARD_SIZE][BOARD_SIZE];
    ChessType m_chessType;
    AIType m_difficulty;
    static constexpr int INF = 1000000;

    // Zobrist 哈希相关
    quint64 zobristTable[BOARD_SIZE][BOARD_SIZE][2];
    quint64 currentHash;
    struct HashEntry { int depth; int score; };
    QMap<quint64, HashEntry> transpositionTable;

    void initZobrist();
    void updateHash(int r, int c, ChessType type);
    int evaluateForPlayer(ChessType color) const;
    void makeMove(int r, int c, ChessType type);
    void unmakeMove(int r, int c, ChessType type);
};

#endif // AITHINKWORKER_H
