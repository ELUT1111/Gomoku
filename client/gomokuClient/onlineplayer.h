#ifndef ONLINEPLAYER_H
#define ONLINEPLAYER_H
#include "abstractplayer.h"
#include "NetworkManager.h"
#include <QString>

class OnlinePlayer : public AbstractPlayer
{
    Q_OBJECT
public:
    explicit OnlinePlayer(QObject *parent = nullptr, ChessType chessType = ChessType::BLACK);

    void setOnlinePlayerTag(const QString& tag) { m_onlineTag = tag; }
    QString getOnlinePlayerTag() const { return m_onlineTag; }

public slots:
    // 重写父类方法：处理自身落子（触发网络发送）
    void slot_onMouseClicked(int x, int y) override;
    // 处理服务端推送的对手落子（触发本地绘子）
    void slot_onOpponentMoveReceived(int x, int y, int color);

private:
    // 在线标识
    QString m_onlineTag;
    NetworkManager& m_netManager = NetworkManager::instance();
};
#endif // ONLINEPLAYER_H
