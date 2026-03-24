#include "OnlinePlayer.h"
#include <qDebug>

OnlinePlayer::OnlinePlayer(QObject *parent, ChessType chessType)
    : AbstractPlayer{parent, chessType}
{
    // 初始化在线标识
    m_onlineTag = (chessType == ChessType::BLACK) ? "BLACK" : "WHITE";
    qDebug() << "[OnlinePlayer] 初始化在线玩家：" << m_onlineTag;
}

void OnlinePlayer::setChessType(ChessType newType)
{
    myChessType = newType;
    m_onlineTag = (newType == ChessType::BLACK) ? "BLACK" : "WHITE";
    qDebug() << "[OnlinePlayer] 玩家棋子颜色已更新为：" << m_onlineTag;
}

// 自身落子→发送到服务端
void OnlinePlayer::slot_onMouseClicked(int x, int y)
{
    qDebug() << "[OnlinePlayer] 自身尝试落子：" << x << "," << y << "，标识：" << m_onlineTag;
    // 发送落子信息到服务端
    m_netManager.sendChessMove(x, y, m_onlineTag);
}

// 处理对手落子→触发本地绘子
void OnlinePlayer::slot_onOpponentMoveReceived(int x, int y, int color)
{
    ChessType oppoChessType = (color == 1) ? ChessType::BLACK : ChessType::WHITE;
    qDebug() << "[OnlinePlayer] 收到对手落子：" << x << "," << y << "，颜色：" << color;
    // 触发本地绘子
    emit signal_tryPlaceChess(x, y, oppoChessType);
}
