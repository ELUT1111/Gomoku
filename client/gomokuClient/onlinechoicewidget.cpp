#include "onlinechoicewidget.h"
#include "ui_onlinechoicewidget.h"
#include "pagemanager.h"
#include "NetworkManager.h"
#include <QMessageBox>

QString g_currentRoomId = "";
QString g_myOnlineTag = "";


OnlineChoiceWidget::OnlineChoiceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineChoiceWidget)
{
    ui->setupUi(this);
    NetworkManager::instance().connectToServer("ws://localhost:8080/gomoku/ws");
    // 绑定网络连接成功信号
    connect(&NetworkManager::instance(), &NetworkManager::connected, this, [](){
        QMessageBox::information(nullptr, "网络提示", "已成功连接到五子棋服务端！");
    });
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
    connect(&NetworkManager::instance(), &NetworkManager::sig_roomInfoReceived, this, [this](QString roomId, QString player, QString msg){
        Q_UNUSED(player);
        Q_UNUSED(msg);
        qDebug() << "[OnlineChoice] 创建房间成功，ID：" << roomId;
        PageManager::instance()->switchToPage(3); // 跳转到OnlineRoomWidget
        // 传递房间ID给房间页面（通过全局变量/单例，此处用全局变量示例，可优化为单例）
        extern QString g_currentRoomId;
        g_currentRoomId = roomId.trimmed().toLower();
        extern QString g_myOnlineTag;
        g_myOnlineTag = player;
    });
}


void OnlineChoiceWidget::on_JoinRoomButton_clicked()
{
    QString roomId = ui->lineEdit_roomid->text().trimmed(); // 新增的房间ID输入框
    if(roomId.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请输入房间ID！");
        return;
    }
    NetworkManager::instance().sendJoinRoom(roomId);
    // 绑定房间信息信号→跳转到房间页面
    connect(&NetworkManager::instance(), &NetworkManager::sig_roomInfoReceived, this, [this](QString roomId, QString player, QString msg){
        Q_UNUSED(roomId);
        QMessageBox::information(this, "房间提示", msg);
        PageManager::instance()->switchToPage(3); // 跳转到OnlineRoomWidget
        extern QString g_myOnlineTag;
        g_myOnlineTag = player;
    });
}


void OnlineChoiceWidget::on_searchGameButton_clicked()
{
    QMessageBox::information(this, "提示", "随机匹配功能正在开发中，敬请期待！");
}

