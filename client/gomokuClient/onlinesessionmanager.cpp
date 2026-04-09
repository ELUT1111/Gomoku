#include "onlinesessionmanager.h"
#include "qdebug.h"

OnlineSessionManager* OnlineSessionManager::m_instance = nullptr;
QString OnlineSessionManager::currentRoomId = "";
QString OnlineSessionManager::myOnlineColor = "";

OnlineSessionManager::OnlineSessionManager(QObject *parent)
    : QObject{parent}
{
    onlineTimer = new QTimer(this);
    onlineTimer->setInterval(1000);

    connect(onlineTimer,&QTimer::timeout,this,&OnlineSessionManager::slot_onTimerTimeout);
}

QString OnlineSessionManager::getCurrentThinkingPlayer() const
{
    return currentThinkingPlayer;
}

void OnlineSessionManager::setCurrentThinkingPlayer(const QString &newCurrentThinkingPlayer)
{
    currentThinkingPlayer = newCurrentThinkingPlayer;
}

void OnlineSessionManager::slot_onTimerTimeout()
{
    elapsedSeconds++;
    emit signal_updateThinkingTime(elapsedSeconds);
}

void OnlineSessionManager::slot_setTimerStatus(bool status)
{
    if(onlineTimer)
    {
        elapsedSeconds = 0;
        if(status)
        {
            onlineTimer->start();
        }
        else
        {
            onlineTimer->stop();
        }
    }

}


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

void OnlineSessionManager::switchCurrentThinkingPlayer()
{
    if(this->currentThinkingPlayer == "BLACK") setCurrentThinkingPlayer("WHITE");
    else setCurrentThinkingPlayer("BLACK");
}