#include "onlineroomlistwidget.h"
#include "ui_onlineroomlistwidget.h"
#include "NetworkManager.h"
#include "pagemanager.h"
#include <QMessageBox>
#include <QTimer>

OnlineRoomListWidget::OnlineRoomListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineRoomListWidget)
{
    ui->setupUi(this);

    // 绑定网络信号：接收房间列表
    connect(&NetworkManager::instance(), &NetworkManager::sig_roomListReceived,
            this, &OnlineRoomListWidget::slotOnRoomListReceived);

    // 初始隐藏空提示
    ui->lblEmptyTip->hide();
    // 首次自动刷新
    on_btnRefresh_clicked();
}

OnlineRoomListWidget::~OnlineRoomListWidget()
{
    clearRoomList();
    delete ui;
}

void OnlineRoomListWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    on_btnRefresh_clicked();
}

void OnlineRoomListWidget::on_btnRefresh_clicked()
{
    // 发送刷新请求
    NetworkManager::instance().sendRefreshRoomList();
    ui->btnRefresh->setEnabled(false);
    ui->btnRefresh->setText("刷新中...");
    // 1秒后恢复按钮
    QTimer::singleShot(1000, this, [this](){
        ui->btnRefresh->setEnabled(true);
        ui->btnRefresh->setText("刷新");
    });
}

void OnlineRoomListWidget::slotOnRoomListReceived(const QJsonArray &roomList)
{
    clearRoomList();
    if(roomList.isEmpty()) {
        ui->lblEmptyTip->show();
        return;
    }
    ui->lblEmptyTip->hide();

    // 遍历渲染房间项
    for(const QJsonValue& val : roomList) {
        QJsonObject obj = val.toObject();
        QString roomId = obj["roomId"].toString();
        QString status = obj["status"].toString();
        int playerCount = obj["playerCount"].toInt();
        addRoomItem(roomId, status, playerCount);
    }
}

void OnlineRoomListWidget::addRoomItem(const QString &roomId, const QString &status, int playerCount)
{
    QListWidgetItem* item = new QListWidgetItem(ui->listWidgetRooms);
    RoomItemWidget* itemWidget = new RoomItemWidget();
    itemWidget->setRoomData(roomId, status, playerCount);

    // 设置项大小
    item->setSizeHint(itemWidget->sizeHint());
    ui->listWidgetRooms->addItem(item);
    ui->listWidgetRooms->setItemWidget(item, itemWidget);

    // 绑定加入信号
    connect(itemWidget, &RoomItemWidget::sigJoinRoom, this, &OnlineRoomListWidget::slotJoinRoom);
}

void OnlineRoomListWidget::slotJoinRoom(QString roomId)
{
    NetworkManager::instance().sendJoinRoom(roomId);
    // 绑定加入成功→跳转到房间页面
    QMetaObject::Connection conn = connect(&NetworkManager::instance(), &NetworkManager::sig_joinSuccessReceived, this,
                                           [this, conn](QString roomId, QString player, QString msg){
                                               QObject::disconnect(conn);
                                               PageManager::instance()->switchToPage(3); // 跳转到在线房间页
                                           });
}

void OnlineRoomListWidget::clearRoomList()
{
    ui->listWidgetRooms->clear();
    // 清理所有项
    QListWidgetItem* item;
    while((item = ui->listWidgetRooms->takeItem(0))) {
        delete item;
    }
}

void OnlineRoomListWidget::on_btnBack_clicked()
{
    PageManager::instance()->switchToPage(2); // 返回在线选择页
}