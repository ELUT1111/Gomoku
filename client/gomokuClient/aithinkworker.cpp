#include "aithinkworker.h"

#include <GameSession.h>
#include <QRandomGenerator>

// const int WIN_SCORE = 99999;
// const int SCORE_AI_OPEN_FOUR = 10000;
// const int SCORE_AI_BLOCKED_FOUR = 5000;
// const int SCORE_AI_OPEN_THREE = 1000;
// const int SCORE_AI_BLOCKED_THREE = 300;
// const int SCORE_AI_OPEN_TWO = 100;
// const int SCORE_AI_BLOCKED_TWO = 10;
// const int SCORE_OPP_OPEN_FOUR = -20000;
// const int SCORE_OPP_BLOCKED_FOUR = -10000;
// const int SCORE_OPP_OPEN_THREE = -2000;
// const int SCORE_OPP_BLOCKED_THREE = -600;
// const int SCORE_OPP_OPEN_TWO = -200;
// const int SCORE_OPP_BLOCKED_TWO = -20;

const int SCORE_FIVE = 100000;
const int SCORE_ALIVE_FOUR = 10000;
const int SCORE_BLOCKED_FOUR = 1000;
const int SCORE_ALIVE_THREE = 1000;
const int SCORE_BLOCKED_THREE = 100;
const int SCORE_ALIVE_TWO = 100;
const int SCORE_BLOCKED_TWO = 10;

const int dirs[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

AIThinkWorker::AIThinkWorker(QObject *parent, ChessType chessType, AIType difficulty)
    : QObject(parent), m_chessType(chessType), m_difficulty(difficulty)
{
    initZobrist();
}

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

    auto bd = GameSession::instance()->boardData;
    currentHash = 0;
    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            ChessType type = bd->getChess(i, j);
            m_localBoard[i][j] = static_cast<int>(type);
            if(type != ChessType::EMPTY) updateHash(i, j, type);
        }
    }

    transpositionTable.clear();
    QPoint pos = nextPos();
    emit thinkFinished(pos);
}

QPoint AIThinkWorker::nextPos()
{
    QPoint threat = checkImmediateThreat();
    if(threat.x() != -1) return threat;

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
    QPoint decision = minimaxDecision(3);
    return (decision.x() < 0) ? randomEmptyPos() : decision;
}

QPoint AIThinkWorker::nextPosForHard()
{
    QPoint decision = minimaxDecision(4);
    return (decision.x() < 0) ? randomEmptyPos() : decision;
}

QPoint AIThinkWorker::checkImmediateThreat()
{
    auto bd = GameSession::instance()->boardData;
    ChessType ai = m_chessType;
    ChessType opp = EnumUtils::oppo(ai);

    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            if(!bd->isPosEmpty(i,j)) continue;
            bd->setChess(i,j,ai);
            bool win = false;
            for(int d = 0; d < 4; ++d) {
                if(bd->numInRow(i,j,ai,d) >=5) {
                    win = true;
                    break;
                }
            }
            bd->setChess(i,j,ChessType::EMPTY);
            if(win) return QPoint(i,j);
        }
    }

    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            if(!bd->isPosEmpty(i,j)) continue;
            bd->setChess(i,j,opp);
            bool win = false;
            for(int d = 0; d < 4; ++d) {
                if(bd->numInRow(i,j,opp,d) >=5) {
                    win = true;
                    break;
                }
            }
            bd->setChess(i,j,ChessType::EMPTY);
            if(win) return QPoint(i,j);
        }
    }

    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            if(!bd->isPosEmpty(i,j)) continue;
            bd->setChess(i,j,opp);
            bool hasOpenFour = false;
            for(int d = 0; d < 4; ++d) {
                int len = bd->numInRow(i,j,opp,d);
                if(len >=4) {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int x1 = i - dx, y1 = j - dy;
                    int x2 = i + len*dx, y2 = j + len*dy;
                    bool open = (x1 >=0 && x1 < BOARD_SIZE && y1 >=0 && y1 < BOARD_SIZE && bd->isPosEmpty(x1,y1)) &&
                                (x2 >=0 && x2 < BOARD_SIZE && y2 >=0 && y2 < BOARD_SIZE && bd->isPosEmpty(x2,y2));
                    if(open) {
                        hasOpenFour = true;
                        break;
                    }
                }
            }
            bd->setChess(i,j,ChessType::EMPTY);
            if(hasOpenFour) return QPoint(i,j);
        }
    }

    // 4. Check opponent's live three (MUST BLOCK, fixes your issue!)
    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            if(!bd->isPosEmpty(i,j)) continue;
            bd->setChess(i,j,opp);
            bool hasOpenThree = false;
            for(int d = 0; d < 4; ++d) {
                int len = bd->numInRow(i,j,opp,d);
                if(len >=3) {
                    int dx = dirs[d][0], dy = dirs[d][1];
                    int x1 = i - dx, y1 = j - dy;
                    int x2 = i + len*dx, y2 = j + len*dy;
                    bool open = (x1 >=0 && x1 < BOARD_SIZE && y1 >=0 && y1 < BOARD_SIZE && bd->isPosEmpty(x1,y1)) &&
                                (x2 >=0 && x2 < BOARD_SIZE && y2 >=0 && y2 < BOARD_SIZE && bd->isPosEmpty(x2,y2));
                    if(open) {
                        hasOpenThree = true;
                        break;
                    }
                }
            }
            bd->setChess(i,j,ChessType::EMPTY);
            if(hasOpenThree) return QPoint(i,j);
        }
    }

    return QPoint(-1,-1);
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
    return emptyPos.at(idx);
}

QList<QPoint> AIThinkWorker::generateCandidateMoves() const
{
    QList<QPoint> moves;
    auto bd = GameSession::instance()->boardData;
    bool hasPushed[BOARD_SIZE][BOARD_SIZE] = {false};

    // 启发式：只搜索已有棋子周围2格范围内的空位
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (bd->getChess(i, j) == ChessType::EMPTY) continue;

            for (int dx = -2; dx <= 2; ++dx) {
                for (int dy = -2; dy <= 2; ++dy) {
                    int nx = i + dx, ny = j + dy;
                    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                        bd->isPosEmpty(nx, ny) && !hasPushed[nx][ny]) {
                        moves.append(QPoint(nx, ny));
                        hasPushed[nx][ny] = true;
                    }
                }
            }
        }
    }

    if (moves.isEmpty()) moves.append(QPoint(BOARD_SIZE/2, BOARD_SIZE/2));
    return moves;
}

int AIThinkWorker::evaluateBoard() const
{
    int score = evaluateForPlayer(m_chessType);
    int oppScore = evaluateForPlayer(EnumUtils::oppo(m_chessType));

    return score - static_cast<int>(oppScore * 1.5);
}

int AIThinkWorker::minimax(int depth, int alpha, int beta, ChessType currentPlayer)
{
    if (transpositionTable.contains(currentHash) && transpositionTable[currentHash].depth >= depth) {
        return transpositionTable[currentHash].score;
    }

    int eval = evaluateBoard();
    if (depth == 0 || qAbs(eval) >= SCORE_FIVE) return eval;

    QList<QPoint> moves = generateCandidateMoves();

    if (currentPlayer == m_chessType) {
        int maxEval = -INF;
        for (const QPoint &mv : moves) {
            makeMove(mv.x(), mv.y(), currentPlayer);
            int score = minimax(depth - 1, alpha, beta, EnumUtils::oppo(currentPlayer));
            unmakeMove(mv.x(), mv.y(), currentPlayer);
            maxEval = qMax(maxEval, score);
            alpha = qMax(alpha, score);
            if (beta <= alpha) break;
        }
        transpositionTable[currentHash] = {depth, maxEval};
        return maxEval;
    } else {
        int minEval = INF;
        for (const QPoint &mv : moves) {
            makeMove(mv.x(), mv.y(), currentPlayer);
            int score = minimax(depth - 1, alpha, beta, EnumUtils::oppo(currentPlayer));
            unmakeMove(mv.x(), mv.y(), currentPlayer);
            minEval = qMin(minEval, score);
            beta = qMin(beta, score);
            if (beta <= alpha) break;
        }
        transpositionTable[currentHash] = {depth, minEval};
        return minEval;
    }
}

QPoint AIThinkWorker::minimaxDecision(int depth)
{
    QList<QPoint> moves = generateCandidateMoves();
    if(moves.isEmpty()) return QPoint(BOARD_SIZE/2, BOARD_SIZE/2);

    QPoint bestMove = moves[0];
    int bestScore = -INF;
    int alpha = -INF;
    int beta = INF;

    for(const QPoint &mv : moves) {
        makeMove(mv.x(), mv.y(), m_chessType);
        int score = minimax(depth - 1, alpha, beta, EnumUtils::oppo(m_chessType));
        unmakeMove(mv.x(), mv.y(), m_chessType);

        if(score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
        alpha = qMax(alpha, bestScore);
    }
    return bestMove;
}

void AIThinkWorker::initZobrist()
{
    QRandomGenerator *gen = QRandomGenerator::global();
    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j) {
            zobristTable[i][j][0] = gen->generate64(); // AI
            zobristTable[i][j][1] = gen->generate64(); // Opponent
        }
    }
}

void AIThinkWorker::updateHash(int r, int c, ChessType type)
{
    if (type == ChessType::EMPTY) return;
    int pIdx = (type == m_chessType) ? 0 : 1;
    currentHash ^= zobristTable[r][c][pIdx];
}

int AIThinkWorker::evaluateForPlayer(ChessType color) const
{
    auto bd = GameSession::instance()->boardData;
    int total = 0;

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (bd->getChess(i, j) != color) continue;

            for (int d = 0; d < 4; ++d) {
                int count = 1;
                // 向前探测
                int r = i + dirs[d][0], c = j + dirs[d][1];
                while(r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && bd->getChess(r,c) == color) {
                    count++;
                    r += dirs[d][0]; c += dirs[d][1];
                }

                // 仅统计线段起点，避免重复计算
                int prevR = i - dirs[d][0], prevC = j - dirs[d][1];
                if(prevR >= 0 && prevR < BOARD_SIZE && prevC >= 0 && prevC < BOARD_SIZE && bd->getChess(prevR, prevC) == color)
                    continue;

                // 检查两端阻塞
                bool headOpen = (prevR >= 0 && prevR < BOARD_SIZE && prevC >= 0 && prevC < BOARD_SIZE && bd->isPosEmpty(prevR, prevC));
                bool tailOpen = (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && bd->isPosEmpty(r,c));

                if (count >= 5) total += SCORE_FIVE;
                else if (count == 4) total += (headOpen && tailOpen) ? SCORE_ALIVE_FOUR : ((headOpen || tailOpen) ? SCORE_BLOCKED_FOUR : 0);
                else if (count == 3) total += (headOpen && tailOpen) ? SCORE_ALIVE_THREE : ((headOpen || tailOpen) ? SCORE_BLOCKED_THREE : 0);
                else if (count == 2) total += (headOpen && tailOpen) ? SCORE_ALIVE_TWO : ((headOpen || tailOpen) ? SCORE_BLOCKED_TWO : 0);
            }
        }
    }
    return total;
}

void AIThinkWorker::makeMove(int r, int c, ChessType type)
{
    updateHash(r, c, type);
    m_localBoard[r][c] = static_cast<int>(type);
}

void AIThinkWorker::unmakeMove(int r, int c, ChessType type)
{
    updateHash(r, c, type);
    m_localBoard[r][c] = static_cast<int>(ChessType::EMPTY);
}
