#include "onlineroomwidget.h"
#include "ui_onlineroomwidget.h"

#include <PageManager.h>
#include <NetworkManager.h>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include "GameSession.h"

// 全局变量（临时传递，后续可封装为配置单例，不修改现有标识符）
extern QString g_currentRoomId;
extern QString g_myOnlineTag;

OnlineRoomWidget::OnlineRoomWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineRoomWidget)
{
    ui->setupUi(this);

    // 初始化：展示房间ID
    if(!g_currentRoomId.isEmpty())
    {
        QString displayRoomId = g_currentRoomId.trimmed().toLower();
        ui->lblRoomId->setText(displayRoomId);
        qDebug() << "[OnlineRoom] 显示房间ID：" << displayRoomId;
    }
    // 初始化：设置房间状态为「等待对手加入」
    ui->lblRoomStatus->setText("等待对手加入...");
    // 绑定服务端房间信息信号→更新房间状态
    connect(&NetworkManager::instance(), &NetworkManager::sig_roomInfoReceived, this, [this](QString roomId, QString player, QString msg){
        Q_UNUSED(player);
        ui->lblRoomStatus->setText(msg);
        // 对手加入后，启用开始游戏按钮
        if(msg.contains("游戏开始"))
        {
            ui->btnStartGame->setEnabled(true);
            ui->lblRoomStatus->setText("对手已加入，点击开始游戏！");
        }
    });
}

OnlineRoomWidget::~OnlineRoomWidget()
{
    delete ui;
}

void OnlineRoomWidget::on_btnQuitRoom_clicked()
{
    PageManager::instance()->switchToPage(2);
    // 重置全局变量
    g_currentRoomId = "";
    g_myOnlineTag = "";
}


void OnlineRoomWidget::on_btnCopyRoomId_clicked()
{
    QString roomId = ui->lblRoomId->text();
    QApplication::clipboard()->setText(roomId);
    QMessageBox::information(this, "提示", "房间ID已复制到剪贴板！");
}


void OnlineRoomWidget::on_btnStartGame_clicked()
{
    // 1. 设置游戏模式为在线
    GameSession::instance()->slot_changeGamemode(GamemodeType::ONLINE);
    // 2. 初始化在线玩家标识（对接服务端）
    ChessType myChessType = (g_myOnlineTag == "BLACK") ? ChessType::BLACK : ChessType::WHITE;
    GameSession::instance()->setOnlinePlayerTag(myChessType, g_myOnlineTag);
    // 3. 跳转到游戏页面
    PageManager::instance()->switchToPage(4);
    // 4. 禁用开始游戏按钮
    ui->btnStartGame->setEnabled(false);
}

