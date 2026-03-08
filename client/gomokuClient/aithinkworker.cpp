#include "aithinkworker.h"

#include <GameSession.h>
#include <QRandomGenerator>

AIThinkWorker::AIThinkWorker(QObject *parent, ChessType chessType, AIType difficulty)
    : QObject(parent), m_chessType(chessType), m_difficulty(difficulty)
{}

void AIThinkWorker::setDifficulty(AIType difficulty)
{
    m_difficulty = difficulty;
}

void AIThinkWorker::setChessType(ChessType chessType)
{
    m_chessType = chessType;
}

void AIThinkWorker::doThink()
{
    emit thinkStarted();
    QPoint pos = nextPos();
    emit thinkFinished(pos);
}

QPoint AIThinkWorker::nextPos()
{
    QPoint nextPoint;
    if(m_difficulty == AIType::EASY)
    {
        nextPoint = nextPosForEasy();
    }
    else if(m_difficulty == AIType::NORMAL)
    {
        nextPoint = nextPosForNormal();
    }
    else
    {
        nextPoint = nextPosForHard();
    }
    return nextPoint;
}

QPoint AIThinkWorker::nextPosForEasy()
{
    // 基础进攻+防守
    for(int i = 0;i < BOARD_SIZE;i++)
    {
        for(int j = 0;j< BOARD_SIZE;j++)
        {
            if(GameSession::instance()->boardData->isPosEmpty(i,j))
            {
                GameSession::instance()->boardData->setChess(i,j,m_chessType);
                for(int k = 0;k < 4;k++)
                {
                    if(GameSession::instance()->boardData->numInRow(i,j,m_chessType,k)>=5)
                    {
                        // 可获胜的落子处
                        GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
                        return QPoint(i,j);
                    }
                }
                // GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
                GameSession::instance()->boardData->setChess(i,j,EnumUtils::oppo(m_chessType));
                for(int k = 0;k < 4;k++)
                {
                    if(GameSession::instance()->boardData->numInRow(i,j,EnumUtils::oppo(m_chessType),k)>=5)
                    {
                        // 堵塞 >=4 的连子
                        GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
                        return QPoint(i,j);
                    }
                }
                GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
            }
        }
    }
    // 二阶进攻
    for(int i = 0;i < BOARD_SIZE;i++)
    {
        for(int j = 0;j< BOARD_SIZE;j++)
        {
            if(GameSession::instance()->boardData->isPosEmpty(i,j))
            {
                GameSession::instance()->boardData->setChess(i,j,m_chessType);
                for(int k = 0;k < 4;k++)
                {
                    if(GameSession::instance()->boardData->numInRow(i,j,m_chessType,k)>=4)
                    {
                        // 可能导致获胜的落子处
                        GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
                        return QPoint(i,j);
                    }
                }

                GameSession::instance()->boardData->setChess(i,j,EnumUtils::oppo(m_chessType));
                for(int k = 0;k < 4;k++)
                {
                    if(GameSession::instance()->boardData->numInRow(i,j,EnumUtils::oppo(m_chessType),k) >= 4)
                    {
                        // 堵塞 >=3 的连子
                        GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
                        return QPoint(i,j);
                    }
                }
                GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
            }
        }
    }
    return randomEmptyPos();
}

QPoint AIThinkWorker::nextPosForNormal()
{
    QPoint decision = minimaxDecision(2);
    if(decision.x() < 0)
        return randomEmptyPos();
    return decision;
}

QPoint AIThinkWorker::nextPosForHard()
{
    QPoint decision = minimaxDecision(4);
    if(decision.x() < 0)
        return randomEmptyPos();
    return decision;
}

QPoint AIThinkWorker::randomEmptyPos()
{
    QList<QPoint> emptyPos;
    for(int i = 0;i < BOARD_SIZE; i++)
    {
        for(int j = 0;j < BOARD_SIZE;j++)
        {
            if(GameSession::instance()->boardData->getChess(i,j) == ChessType::EMPTY)
            {
                emptyPos.append(QPoint(i,j));
            }
        }
    }
    if(emptyPos.isEmpty()) return QPoint(-1,-1);
    int idx = QRandomGenerator::global()->bounded(emptyPos.size());
    //QPoint randomPoint = emptyPos.at(idx);
    // qDebug()<<"[AIPlayer_EASY] 随机落子"<<randomPoint;
    return emptyPos.at(idx);
}

QList<QPoint> AIThinkWorker::generateCandidateMoves() const
{
    QList<QPoint> moves;
    auto bd = GameSession::instance()->boardData;
    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            if(bd->isPosEmpty(i, j)) {
                bool near = false;
                for(int dx = -2; dx <= 2 && !near; ++dx) {
                    for(int dy = -2; dy <= 2 && !near; ++dy) {
                        if(dx == 0 && dy == 0) continue;
                        int nx = i + dx;
                        int ny = j + dy;
                        if(nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) continue;
                        if(bd->getChess(nx, ny) != ChessType::EMPTY) {
                            near = true;
                        }
                    }
                }
                if(near) moves.append(QPoint(i, j));
            }
        }
    }
    if(moves.isEmpty()) {
        // every position is candidate
        for(int i = 0; i < BOARD_SIZE; ++i)
            for(int j = 0; j < BOARD_SIZE; ++j)
                if(GameSession::instance()->boardData->isPosEmpty(i,j))
                    moves.append(QPoint(i,j));
    }
    return moves;
}

int AIThinkWorker::evaluateBoard() const
{
    auto bd = GameSession::instance()->boardData;
    // check immediate win/loss
    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            ChessType t = bd->getChess(i, j);
            if(t == ChessType::EMPTY) continue;
            for(int dir = 0; dir < 4; ++dir) {
                int len = bd->numInRow(i, j, t, dir);
                if(len >= 5) {
                    if(t == m_chessType)
                        return INF;
                    else
                        return -INF;
                }
            }
        }
    }

    int score = 0;
    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            ChessType t = bd->getChess(i, j);
            if(t == ChessType::EMPTY) continue;
            for(int dir = 0; dir < 4; ++dir) {
                int len = bd->numInRow(i, j, t, dir);
                int value = len * len;
                if(t == m_chessType)
                    score += value;
                else
                    score -= value;
            }
        }
    }
    return score;
}

int AIThinkWorker::minimax(int depth, int alpha, int beta, ChessType currentPlayer)
{
    // terminal or depth limit
    int eval = evaluateBoard();
    if(depth == 0 || eval == INF || eval == -INF) {
        return eval;
    }

    QList<QPoint> moves = generateCandidateMoves();
    if(moves.isEmpty())
        return 0;

    if(currentPlayer == m_chessType) {
        int maxEval = -INF;
        for(const QPoint &mv : moves) {
            GameSession::instance()->boardData->setChess(mv.x(), mv.y(), currentPlayer);
            int score = minimax(depth - 1, alpha, beta, EnumUtils::oppo(currentPlayer));
            GameSession::instance()->boardData->setChess(mv.x(), mv.y(), ChessType::EMPTY);
            maxEval = qMax(maxEval, score);
            alpha = qMax(alpha, score);
            if(beta <= alpha)
                break;
        }
        return maxEval;
    } else {
        int minEval = INF;
        for(const QPoint &mv : moves) {
            GameSession::instance()->boardData->setChess(mv.x(), mv.y(), currentPlayer);
            int score = minimax(depth - 1, alpha, beta, EnumUtils::oppo(currentPlayer));
            GameSession::instance()->boardData->setChess(mv.x(), mv.y(), ChessType::EMPTY);
            minEval = qMin(minEval, score);
            beta = qMin(beta, score);
            if(beta <= alpha)
                break;
        }
        return minEval;
    }
}

QPoint AIThinkWorker::minimaxDecision(int depth)
{
    QList<QPoint> moves = generateCandidateMoves();
    if(moves.isEmpty())
        return QPoint(-1, -1);

    QPoint bestMove(-1, -1);
    int bestScore = -INF;
    for(const QPoint &mv : moves) {
        GameSession::instance()->boardData->setChess(mv.x(), mv.y(), m_chessType);
        int score = minimax(depth - 1, -INF, INF, EnumUtils::oppo(m_chessType));
        GameSession::instance()->boardData->setChess(mv.x(), mv.y(), ChessType::EMPTY);
        if(score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
    }
    return bestMove;
}
