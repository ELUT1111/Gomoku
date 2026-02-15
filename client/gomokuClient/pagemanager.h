#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include "EnumType.h"
#include <QObject>
#include <QMutex>
#include <QMutexLocker>

class PageManager : public QObject
{
    Q_OBJECT
    // 单例工具类，为页面跳转提供统一接口
    static PageManager* pageManager;
    static QMutex mutex;
public:
    explicit PageManager(QObject *parent = nullptr);
    static PageManager* instance()
    {
        if(!pageManager)
        {
            QMutexLocker locker(&mutex);
            if(!pageManager)
            {
                pageManager = new PageManager();
                return pageManager;
            }
        }
        return pageManager;
    }
    void switchToPage(int i)
    {
        emit signal_switchToPage(i);
    }
    static void destroyInstance()
    {
        QMutexLocker locker(&mutex);
        if(pageManager)
        {
            delete pageManager;
            pageManager = nullptr;
        }
    }

signals:
    void signal_switchToPage(int index);
    void signal_changeGamemode(GamemodeType gamemode);
};

#endif // PAGEMANAGER_H
