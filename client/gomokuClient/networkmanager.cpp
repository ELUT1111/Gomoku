#include "NetworkManager.h"

#include <OnlineSessionManager.h>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    connect(&m_socket, &QWebSocket::connected, this, &NetworkManager::connected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &NetworkManager::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::disconnected, this, &NetworkManager::onSocketDisconnected);
    connect(&m_socket, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error){
        emit errorOccurred("网络错误：" + m_socket.errorString());
    });
}

void NetworkManager::connectToServer(const QString& url) {
    if(isConnected()) {
        qDebug() << "[Network] 已连接到服务端，无需重复连接";
        return;
    }
    m_socket.open(QUrl(url));
    qDebug() << "[Network] 正在连接服务端：" << url;
}
void NetworkManager::sendJoinRoom(const QString& roomId) {
    if(!isConnected()) {
        emit errorOccurred("未连接到服务端，请先连接");
        return;
    }
    m_socket.sendTextMessage(serializeMsg("JOIN_ROOM", roomId));
}

void NetworkManager::sendQuitRoomRequest()
{
    if(!isConnected()) return;
    QJsonObject obj;
    obj["type"] = "QUIT_ROOM";
    obj["roomId"] = OnlineSessionManager::instance()->getCurrentRoomId();
    m_socket.sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    qDebug() << "[Network] 发送退出房间请求：" << OnlineSessionManager::instance()->getCurrentRoomId();
}

// 发送落子
void NetworkManager::sendChessMove(int x, int y, const QString& player) {
    if(!isConnected()) {
        emit errorOccurred("未连接到服务端，落子失败");
        return;
    }
    m_socket.sendTextMessage(serializeMsg("CHESS_MOVE", OnlineSessionManager::instance()->getCurrentRoomId(), player, x, y));
}

// 网络断开
void NetworkManager::onSocketDisconnected() {
    qDebug() << "[Network] 与服务端断开连接";
    emit errorOccurred("与服务端断开连接，请重新连接");
}

void NetworkManager::sendCreateRoom() {
    if(!isConnected()) {
        emit errorOccurred("未连接到服务端，请先连接");
        return;
    }
    // 调用序列化方法，发送 CREATE_ROOM 消息（无额外参数）
    m_socket.sendTextMessage(serializeMsg("CREATE_ROOM"));
    qDebug() << "[Network] 发送创建房间请求";
}

void NetworkManager::sendRandomMatchRequest() {
    QJsonObject obj;
    obj["type"] = "RANDOM_MATCH";
    m_socket.sendTextMessage(QJsonDocument(obj).toJson());
}

void NetworkManager::onTextMessageReceived(QString message) {
    qDebug() << "[Network] 收到服务端消息：" << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if(!doc.isObject()) {
        emit errorOccurred("服务端消息格式错误");
        return;
    }
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();

    if (type == "ROOM_INFO") {
        QString roomId = obj["roomId"].toString();
        QString player = obj["player"].toString();
        QString msg = obj["msg"].toString();
        emit sig_roomInfoReceived(roomId, player, msg);
    }else if(type == "CREATE_ROOM_STATUS"){
        QString roomId = obj["roomId"].toString();
        QString player = obj["player"].toString();
        QString msg = obj["msg"].toString();
        bool status = obj["decision"].toBool();
        emit sig_createRoomStatusReceived(roomId,player,status,msg);
    }else if(type == "JOIN_SUCCESS"){
        QString roomId = obj["roomId"].toString();
        QString player = obj["player"].toString();
        QString msg = obj["msg"].toString();
        emit sig_joinSuccessReceived(roomId,player,msg);
    }else if (type == "PLAYER_READY") {
        QString msg = obj["msg"].toString();
        emit sig_playerReadyReceived(obj["decision"].toBool(), msg);
    }else if(type == "ROOM_OWNER_CHANGE"){
        QString roomId = obj["roomId"].toString();
        QString player = obj["player"].toString();
        QString msg = obj["msg"].toString();
        emit sig_roomOwnerChangeReceived(roomId,player,msg);
    }else if(type == "QUIT_ROOM_SUCCESS"){
        QString player = obj["player"].toString();
        QString msg = obj["msg"].toString();
        bool status = obj["decision"].toBool();
        emit sig_quitRoomSuccessReceived(player,msg,status);
    }else if (type == "ROOM_LIST") {
        QByteArray jsonData = obj["msg"].toString().toUtf8();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isArray()) {
            qWarning() << "房间列表JSON格式错误";
            return;
        }
        QJsonArray list = doc.array();
        emit sig_roomListReceived(list);
    }else if (type == "GAME_START") {
        QString msg = obj["msg"].toString();
        emit sig_gameStartReceived(msg);
    }else if (type == "CHESS_MOVE") {
        int x = obj["x"].toInt();
        int y = obj["y"].toInt();
        int color = (obj["player"].toString() == "BLACK") ? 1 : 2;
        emit moveReceived(x, y, color);
    }else if (type == "PLACE_CHESS_STATUS"){
        int x = obj["x"].toInt();
        int y = obj["y"].toInt();
        int color = (obj["player"].toString() == "BLACK") ? 1 : 2;
        bool status = obj["decision"].toBool();
        emit sig_placeChessStatusReceived(x,y,color,status);
    }else if (type == "GAME_OVER") {
        QString msg = obj["msg"].toString();
        emit sig_gameOverReceived(msg);
    } else if (type == "ERROR") {
        QString msg = obj["msg"].toString();
        emit sig_errorReceived(msg);
    }
}

void NetworkManager::sendRefreshRoomList()
{
    if(!isConnected()) return;
    m_socket.sendTextMessage(serializeMsg("REFRESH_ROOM_LIST"));
}

QByteArray NetworkManager::serializeMsg(const QString& type, const QString& roomId,
                                        const QString& player, int x, int y, const QString& msg, bool decision) {
    QJsonObject obj;
    obj["type"] = type;
    if(!roomId.isEmpty()) obj["roomId"] = roomId;
    if(!player.isEmpty()) obj["player"] = player;
    if(x >= 0 && y >= 0) {
        obj["x"] = x;
        obj["y"] = y;
    }
    if(!msg.isEmpty()) obj["msg"] = msg;
    obj["decision"] = decision;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

void NetworkManager::sendReadyRequest(bool isReady) {
    if(!isConnected()) return;
    QJsonObject msg;
    msg["type"] = "PLAYER_READY";
    msg["decision"] = isReady;
    m_socket.sendTextMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
}

void NetworkManager::sendStartGameRequest() {
    if(!isConnected()) return;
    m_socket.sendTextMessage(serializeMsg("START_GAME"));
}

void NetworkManager::sendCancelMatchRequest()
{
    if(!isConnected()) return;
    QJsonObject obj;
    obj["type"] = "CANCEL_MATCH";
    m_socket.sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    qDebug() << "[Network] 发送取消匹配请求";
}
