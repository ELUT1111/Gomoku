#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    static NetworkManager& instance() {
        static NetworkManager _instance;
        return _instance;
    }

    void connectToServer(const QString& url);
    void disconnectFromServer();
    void sendCreateRoom(); // 创建房间
    void sendJoinRoom(const QString& roomId); // 加入房间
    void sendQuitRoomRequest(); //退出房间
    void sendRefreshRoomList(); // 刷新房间列表
    void sendChessMove(int x, int y, const QString& player); // 发送落子
    void sendUndoRequest(bool status);
    void sendUndoChoice(bool choice);
    void sendRandomMatchRequest();
    void sendReadyRequest(bool isReady);
    void sendStartGameRequest();
    void sendCancelMatchRequest(); // 取消匹配
    void sendReplayChoice(bool agreeReplay);

    QString getCurrentSessionId() { return m_sessionId; }
    bool isConnected() { return m_socket.state() == QAbstractSocket::ConnectedState; }

signals:
    void connected();
    void gameStarted(int myColor);    // 1为黑，2为白

    void sig_playerReadyReceived(bool currentState,QString msg);
    void sig_gameStartReceived(QString msg); // 游戏正式开始信号
    void moveReceived(int x, int y, int color);
    void errorOccurred(QString msg);

    void sig_createRoomStatusReceived(QString roomId, QString player,bool status ,QString msg);
    void sig_undoStatusReceived(QString roomId, QString player,bool status ,QString msg);
    void sig_undoRequestReceived(QString roomId, QString player,bool status, QString msg);
    void sig_joinSuccessReceived(QString roomId, QString player,QString msg);
    void sig_roomOwnerChangeReceived(QString roomId, QString player,QString msg);
    void sig_roomInfoReceived(QString roomId, QString player, QString msg); // 房间信息
    void sig_roomListReceived(QJsonArray roomList); // 房间列表信息
    void sig_quitRoomSuccessReceived(QString player,QString msg,bool status);
    void sig_placeChessStatusReceived(int x,int y,int color,bool status);
    void sig_gameOverReceived(QString msg); // 游戏结束
    void sig_errorReceived(QString msg); // 错误信息
    void sig_replayChoiceReceived(QString roomId, QString player, bool status, QString msg); // 对方重开选择
    void sig_replayStartReceived(QString roomId, QString newColor, QString msg); // 重开成功
    void sig_replayCancelReceived(QString roomId, QString player, QString msg); // 重开取消
    void sig_gameOverDisconnectReceived(QString roomId, QString msg);

private slots:
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();

private:
    QWebSocket m_socket;
    QString m_sessionId;
    QByteArray serializeMsg(const QString& type, const QString& roomId = "",
                            const QString& player = "", int x = -1, int y = -1, const QString& msg = "",
                            bool decision = false);
};

#endif
