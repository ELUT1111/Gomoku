#include "pagemanager.h"

PageManager::PageManager(QObject *parent)
    : QObject{parent}
{}

PageManager* PageManager::pageManager = nullptr;
QMutex PageManager::mutex;
