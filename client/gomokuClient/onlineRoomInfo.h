#ifndef ONLINEROOMINFO_H
#define ONLINEROOMINFO_H

#include <QString>

// 房间状态枚举
enum class RoomStatus {
    WAITING,   // 等待玩家
    PLAYING,   // 游戏中
    CLOSED     // 已关闭
};

// 房间信息数据结构
class OnlineRoomInfo {
public:
    QString roomId;          // 房间ID
    QString creator;         // 创建者标识
    int playerCount;         // 当前玩家数（0/1/2）
    RoomStatus status;       // 房间状态
    QString currentPlayer;   // 当前落子玩家
    bool isMeOwner = false;  // 当前客户端房主标识

    OnlineRoomInfo() : playerCount(0), status(RoomStatus::CLOSED) {}

    OnlineRoomInfo(QString id, QString crt, int count, RoomStatus st)
        : roomId(id), creator(crt), playerCount(count), status(st) {}
};
#endif // ONLINEROOMINFO_H
