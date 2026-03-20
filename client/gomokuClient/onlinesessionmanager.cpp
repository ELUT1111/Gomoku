#include "onlinesessionmanager.h"

OnlineSessionManager* OnlineSessionManager::m_instance = nullptr;
QString OnlineSessionManager::currentRoomId = "";
QString OnlineSessionManager::myOnlineColor = "";


OnlineSessionManager::OnlineSessionManager(QObject *parent)
    : QObject{parent}
{}

OnlineSessionManager *OnlineSessionManager::instance()
{
    if(m_instance == nullptr) m_instance = new OnlineSessionManager();
    return m_instance;
}
QString OnlineSessionManager::getCurrentRoomId()
{
    return currentRoomId;
}

void OnlineSessionManager::setCurrentRoomId(const QString &newCurrentRoomId)
{
    currentRoomId = newCurrentRoomId;
}

QString OnlineSessionManager::getMyOnlineColor()
{
    return myOnlineColor;
}
void OnlineSessionManager::setMyOnlineColor(const QString &newMyOnlineColor)
{
    myOnlineColor = newMyOnlineColor;
}