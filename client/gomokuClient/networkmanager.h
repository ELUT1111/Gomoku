#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
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
    void sendCreateRoom(); // 创建房间
    void sendJoinRoom(const QString& roomId); // 加入房间
    void sendChessMove(int x, int y, const QString& player); // 发送落子
    void sendMatchRequest();
    void sendMove(int x, int y, int color);

    QString getCurrentSessionId() { return m_sessionId; }
    bool isConnected() { return m_socket.state() == QAbstractSocket::ConnectedState; }

signals:
    void connected();
    void gameStarted(int myColor);    // 1为黑，2为白
    void moveReceived(int x, int y, int color);
    void errorOccurred(QString msg);

    void sig_roomInfoReceived(QString roomId, QString player, QString msg); // 房间信息
    void sig_gameOverReceived(QString msg); // 游戏结束
    void sig_errorReceived(QString msg); // 错误信息

private slots:
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();

private:
    QWebSocket m_socket;
    QString m_sessionId;
    QByteArray serializeMsg(const QString& type, const QString& roomId = "",
                            const QString& player = "", int x = -1, int y = -1, const QString& msg = "");
};

#endif
