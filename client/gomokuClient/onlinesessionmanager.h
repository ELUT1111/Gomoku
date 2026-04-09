#ifndef ONLINESESSIONMANAGER_H
#define ONLINESESSIONMANAGER_H

#include <QObject>
#include <QTimer>

class OnlineSessionManager : public QObject
{
    Q_OBJECT
    static OnlineSessionManager* m_instance;
    static QString currentRoomId;
    static QString myOnlineColor;
    QString currentThinkingPlayer = "BLACK";
    QTimer* onlineTimer = nullptr;
    int elapsedSeconds;
public:
    explicit OnlineSessionManager(QObject *parent = nullptr);
    static OnlineSessionManager* instance();
    static QString getCurrentRoomId();
    static void setCurrentRoomId(const QString &newCurrentRoomId);

    static QString getMyOnlineColor();

    static void setMyOnlineColor(const QString &newMyOnlineColor);

    void switchCurrentThinkingPlayer();

    QString getCurrentThinkingPlayer() const;
    void setCurrentThinkingPlayer(const QString &newCurrentThinkingPlayer);

private:
signals:
    void signal_setThinkingPlayerTime(int s);
    void signal_updateThinkingTime(int s);

public slots:
    void slot_onTimerTimeout();
    void slot_setTimerStatus(bool status);
};

#endif // ONLINESESSIONMANAGER_H
