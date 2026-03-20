#ifndef ONLINESESSIONMANAGER_H
#define ONLINESESSIONMANAGER_H

#include <QObject>

class OnlineSessionManager : public QObject
{
    Q_OBJECT
    static OnlineSessionManager* m_instance;
    static QString currentRoomId;
    static QString myOnlineColor;
public:
    explicit OnlineSessionManager(QObject *parent = nullptr);
    static OnlineSessionManager* instance();
    static QString getCurrentRoomId();
    static void setCurrentRoomId(const QString &newCurrentRoomId);

    static QString getMyOnlineColor();

    static void setMyOnlineColor(const QString &newMyOnlineColor);

private:
signals:
};

#endif // ONLINESESSIONMANAGER_H
