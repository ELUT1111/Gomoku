#include "onlinechoicewidget.h"
#include "ui_onlinechoicewidget.h"
#include "pagemanager.h"
#include "NetworkManager.h"
#include <OnlineSessionManager.h>
#include <QMessageBox>

OnlineChoiceWidget::OnlineChoiceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineChoiceWidget)
{
    ui->setupUi(this);
    NetworkManager::instance().connectToServer("ws://localhost:8080/gomoku/ws");
    // 绑定网络连接成功信号
    // connect(&NetworkManager::instance(), &NetworkManager::connected, this, [](){
    //     QMessageBox::information(nullptr, "网络提示", "已成功连接到五子棋服务端！");
    // });
    // 绑定网络错误信号
    connect(&NetworkManager::instance(), &NetworkManager::errorOccurred, this, [](QString msg){
        QMessageBox::warning(nullptr, "网络错误", msg);
    });
}

OnlineChoiceWidget::~OnlineChoiceWidget()
{
    delete ui;
}

void OnlineChoiceWidget::on_returnButton_2_clicked()
{
    PageManager::instance()->switchToPage(0);
}


void OnlineChoiceWidget::on_addRoomButton_clicked()
{
    NetworkManager::instance().sendCreateRoom();

    // 绑定房间信息信号→跳转到房间页面
    QSharedPointer<QMetaObject::Connection> conn = QSharedPointer<QMetaObject::Connection>::create();
    *conn = connect(&NetworkManager::instance(), &NetworkManager::sig_roomInfoReceived, this,
                    [conn](QString roomId, QString player, QString msg){
        Q_UNUSED(player);
        Q_UNUSED(msg);

        qDebug() << "[OnlineChoice] 创建房间成功，ID：" << roomId;

        // 传递房间ID给房间页面
        OnlineSessionManager::instance()->setCurrentRoomId(roomId.trimmed().toLower());

        OnlineSessionManager::instance()->setMyOnlineColor("BLACK");

        PageManager::instance()->switchToPage(3); // 跳转到OnlineRoomWidget

        QObject::disconnect(*conn);
    });

}


void OnlineChoiceWidget::on_JoinRoomButton_clicked()
{
    QString inputRoomId = ui->lineEdit_roomid->text().trimmed().toLower(); // 统一小写
    if(inputRoomId.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请输入房间ID！");
        return;
    }
    NetworkManager::instance().sendJoinRoom(inputRoomId);

    //绑定房间信息信号→跳转到房间页面
    QSharedPointer<QMetaObject::Connection> conn = QSharedPointer<QMetaObject::Connection>::create();
    *conn = connect(&NetworkManager::instance(), &NetworkManager::sig_joinSuccessReceived, this,
                    [conn](QString roomId, QString player, QString msg){
        QObject::disconnect(*conn);
        Q_UNUSED(msg);
        Q_UNUSED(player);
        OnlineSessionManager::instance()->setCurrentRoomId(roomId.trimmed().toLower());
        OnlineSessionManager::instance()->setMyOnlineColor("WHITE");
        PageManager::instance()->switchToPage(3); // 跳转到OnlineRoomWidget
    });


}
void OnlineChoiceWidget::on_searchGameButton_clicked()
{
    QMessageBox::information(this, "提示", "随机匹配功能正在开发中，敬请期待！");
}


void OnlineChoiceWidget::on_roomListButton_clicked()
{
    PageManager::instance()->switchToPage(4); // 4=房间列表
}

