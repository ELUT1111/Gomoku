#ifndef ONLINEROOMLISTWIDGET_H
#define ONLINEROOMLISTWIDGET_H

#include <QWidget>
#include <QJsonArray>
#include <QListWidgetItem>
#include "roomitemwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OnlineRoomListWidget; }
QT_END_NAMESPACE

class OnlineRoomListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OnlineRoomListWidget(QWidget *parent = nullptr);
    ~OnlineRoomListWidget();
protected:
    void showEvent(QShowEvent *event) override;
private slots:
    // 刷新按钮
    void on_btnRefresh_clicked();
    // 接收房间列表
    void slotOnRoomListReceived(const QJsonArray& roomList);
    // 点击加入房间
    void slotJoinRoom(QString roomId);
    // 返回按钮
    void on_btnBack_clicked();

private:
    // 清空列表
    void clearRoomList();
    // 添加房间项
    void addRoomItem(const QString& roomId, const QString& status, int playerCount);

private:
    Ui::OnlineRoomListWidget *ui;
};

#endif // ONLINEROOMLISTWIDGET_H