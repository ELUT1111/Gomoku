#include "aiplayer.h"
#include "gamesession.h"
#include "qdebug.h"
#include "qpoint.h"

#include <QRandomGenerator>

AIPlayer::AIPlayer(QObject *parent, ChessType chessType, AIType difficutify)
    : AbstractPlayer{parent, chessType}, difficutify(difficutify)
{}

AIType AIPlayer::getDifficutify() const
{
    return difficutify;
}

void AIPlayer::setDifficutify(AIType newDifficutify)
{
    difficutify = newDifficutify;
}

void AIPlayer::startThinking()
{
    if (isThinking) return;
    isThinking = true;
    QPoint nextPoint = nextPos();
    qDebug()<<"[AIPlayer] 尝试落子"<<nextPoint;
    emit signal_tryPlaceChess(nextPoint.x(), nextPoint.y(), myChessType);
    isThinking = false;
}

void AIPlayer::cancelThinking()
{
    isThinking = false;
}

QPoint AIPlayer::nextPos()
{
    QPoint nextPoint;
    if(difficutify == AIType::EASY)
    {
        nextPoint = nextPosForEasy();
    }
    else if(difficutify == AIType::NORMAL)
    {

    }
    else
    {

    }
    return nextPoint;
}

QPoint AIPlayer::nextPosForEasy()
{
    // 基础进攻+防守
    for(int i = 0;i < BOARD_SIZE;i++)
    {
        for(int j = 0;j< BOARD_SIZE;j++)
        {
            if(GameSession::instance()->boardData->isPosEmpty(i,j))
            {
                GameSession::instance()->boardData->setChess(i,j,myChessType);
                for(int k = 0;k < 4;k++)
                {
                    if(GameSession::instance()->boardData->numInRow(i,j,myChessType,k)>=5)
                    {
                        // 可获胜的落子处
                        GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
                        return QPoint(i,j);
                    }
                }
                // GameSession::instance()->boardData->setChess(i,j,ChessType::EMPTY);
                GameSession::instance()->boardData->setChess(i,j,EnumUtils::oppo(myChessType));
                for(int k = 0;k < 4;k++)
                {
                    if(GameSession::instance()->boardData->numInRow(i,j,EnumUtils::oppo(myChessType),k)>=4)
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
    // 二阶进攻
    for(int i = 0;i < BOARD_SIZE;i++)
    {
        for(int j = 0;j< BOARD_SIZE;j++)
        {
            if(GameSession::instance()->boardData->isPosEmpty(i,j))
            {
                GameSession::instance()->boardData->setChess(i,j,myChessType);
                for(int k = 0;k < 4;k++)
                {
                    if(GameSession::instance()->boardData->numInRow(i,j,myChessType,k)>=4)
                    {
                        // 可能导致获胜的落子处
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

QPoint AIPlayer::nextPosForNormal()
{
    return randomEmptyPos();
}

QPoint AIPlayer::nextPosForHard()
{
    return randomEmptyPos();
}

QPoint AIPlayer::randomEmptyPos()
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

void AIPlayer::slot_onMouseClicked(int x, int y)
{
    return;
}

void AIPlayer::slot_myTurn()
{
    startThinking();
}
