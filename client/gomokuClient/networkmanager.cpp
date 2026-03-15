#include "NetworkManager.h"

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

// 发送落子
void NetworkManager::sendChessMove(int x, int y, const QString& player) {
    if(!isConnected()) {
        emit errorOccurred("未连接到服务端，落子失败");
        return;
    }
    m_socket.sendTextMessage(serializeMsg("CHESS_MOVE", "", player, x, y));
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

void NetworkManager::sendMatchRequest() {
    QJsonObject obj;
    obj["type"] = "MATCH";
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
    }else if (type == "PLAYER_READY") {
        QString msg = obj["msg"].toString();
        emit sig_playerReadyReceived(true, msg);
    }else if (type == "GAME_START") {
        QString msg = obj["msg"].toString();
        emit sig_gameStartReceived(msg);
    }else if (type == "CHESS_MOVE") {
        int x = obj["x"].toInt();
        int y = obj["y"].toInt();
        int color = (obj["player"].toString() == "BLACK") ? 1 : 2;
        emit moveReceived(x, y, color);
    } else if (type == "GAME_OVER") {
        QString msg = obj["msg"].toString();
        emit sig_gameOverReceived(msg);
    } else if (type == "ERROR") {
        QString msg = obj["msg"].toString();
        emit sig_errorReceived(msg);
    }
}

QByteArray NetworkManager::serializeMsg(const QString& type, const QString& roomId,
                                        const QString& player, int x, int y, const QString& msg) {
    QJsonObject obj;
    obj["type"] = type;
    if(!roomId.isEmpty()) obj["roomId"] = roomId;
    if(!player.isEmpty()) obj["player"] = player;
    if(x >= 0 && y >= 0) {
        obj["x"] = x;
        obj["y"] = y;
    }
    if(!msg.isEmpty()) obj["msg"] = msg;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

void NetworkManager::sendReadyRequest() {
    if(!isConnected()) return;
    m_socket.sendTextMessage(serializeMsg("PLAYER_READY"));
}

void NetworkManager::sendStartGameRequest() {
    if(!isConnected()) return;
    m_socket.sendTextMessage(serializeMsg("START_GAME"));
}
