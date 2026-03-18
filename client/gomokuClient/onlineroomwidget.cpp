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
        ui->lblRoomId->setText(roomId);
        ui->lblRoomStatus->setText(msg);
    });

    //监听玩家加入、退出
    QSharedPointer<QMetaObject::Connection> conn2 = QSharedPointer<QMetaObject::Connection>::create();
    *conn2 = connect(&NetworkManager::instance(), &NetworkManager::sig_joinSuccessReceived, this,
                     [this](QString roomId, QString player, QString msg){
                         Q_UNUSED(roomId);
                         Q_UNUSED(player);

                        ui->lblOpponentId->setText("已加入");
                        ui->lblRoomStatus->setText(msg);
    });

    //监听玩家准备状态变更
    connect(&NetworkManager::instance(), &NetworkManager::sig_playerReadyReceived, this, [this](bool currentState,QString msg){
        if(currentState)
        {
            qDebug()<<g_myOnlineTag;
            if(g_myOnlineTag == "BLACK")
            {
                ui->btnStartGame->setEnabled(true);
            }
            ui->lblOpponentStatus->setText("已准备");
            ui->lblOpponentStatus->setStyleSheet("color: #4A6CF7");
        }
        else
        {
            if(g_myOnlineTag == "BLACK")
            {
                ui->btnStartGame->setEnabled(false);
            }
            ui->lblOpponentStatus->setText("未准备");
            ui->lblOpponentStatus->setStyleSheet("color: #FF6B6B");
        }
        ui->lblRoomStatus->setText(msg);
    });

    // 监听游戏正式开始
    connect(&NetworkManager::instance(), &NetworkManager::sig_gameStartReceived, this, [this](QString msg){
        // QMessageBox::information(this, "游戏开始", msg);
        GameSession::instance()->slot_changeGamemode(GamemodeType::ONLINE);
        ChessType myChessType = (g_myOnlineTag == "BLACK") ? ChessType::BLACK : ChessType::WHITE;
        GameSession::instance()->setOnlinePlayerTag(myChessType, g_myOnlineTag);
        PageManager::instance()->switchToPage(4);
    });
}

OnlineRoomWidget::~OnlineRoomWidget()
{
    delete ui;
}

void OnlineRoomWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    ui->lblRoomId->setText(g_currentRoomId.trimmed().toLower());
    ui->lblHostChess->setText("棋子：黑色");
    ui->lblOpponentChess->setText("棋子：白色");

    if (g_myOnlineTag == "BLACK") {
        // 房主默认UI
        ui->lblHostId->setText("我 (房主)");
        ui->lblHostStatus->setText("已准备");
        ui->lblOpponentId->setText("未加入");
        ui->lblOpponentStatus->setText("未在线");
        ui->btnStartGame->setText("开始游戏");
        ui->btnStartGame->setEnabled(false);
        ui->lblRoomStatus->setText("等待对手加入...");
    } else {
        // 其他默认UI
        ui->lblHostId->setText("房主");
        ui->lblHostStatus->setText("已准备");
        ui->lblOpponentId->setText("我 (已加入)");
        ui->lblOpponentStatus->setText("未准备");
        ui->btnStartGame->setText("准备");
        ui->btnStartGame->setEnabled(true);
        ui->lblRoomStatus->setText("成功加入房间，请点击准备");
    }
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
    // QMessageBox::information(this, "提示", "房间ID已复制到剪贴板！");
}


void OnlineRoomWidget::on_btnStartGame_clicked()
{
    if (g_myOnlineTag == "BLACK") {
        NetworkManager::instance().sendStartGameRequest(); // 房主点击触发开始
    } else {
        ready = !ready;
        if(getReady())
        {
            ui->btnStartGame->setText("取消准备");
            ui->btnStartGame->setStyleSheet("background-color: #FF6B6B");
            NetworkManager::instance().sendReadyRequest(true); // 点击准备
        }
        else
        {
            ui->btnStartGame->setText("准备");
            ui->btnStartGame->setStyleSheet("background-color: #4A6CF7;");
            NetworkManager::instance().sendReadyRequest(false); //点击了取消准备
        }

    }
}

bool OnlineRoomWidget::getReady() const
{
    return ready;
}

void OnlineRoomWidget::setReady(bool newReady)
{
    ready = newReady;
}
