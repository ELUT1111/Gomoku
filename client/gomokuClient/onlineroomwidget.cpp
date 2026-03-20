#include "onlineroomwidget.h"
#include "ui_onlineroomwidget.h"

#include <PageManager.h>
#include <NetworkManager.h>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <OnlineSessionManager.h>
#include "GameSession.h"


OnlineRoomWidget::OnlineRoomWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineRoomWidget)
{
    ui->setupUi(this);

    // 初始化：展示房间ID
    if(!OnlineSessionManager::instance()->getCurrentRoomId().isEmpty())
    {
        QString displayRoomId = OnlineSessionManager::instance()->getCurrentRoomId().trimmed().toLower();
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

    QSharedPointer<QMetaObject::Connection> conn3 = QSharedPointer<QMetaObject::Connection>::create();
    *conn3 = connect(&NetworkManager::instance(),&NetworkManager::sig_quitRoomSuccessReceived,this,[this](QString player,QString msg,bool status)
        {
        if(player == OnlineSessionManager::instance()->getMyOnlineColor())
        {
            // 断开所有房间相关信号
            disconnect(&NetworkManager::instance(), &NetworkManager::sig_roomInfoReceived, this, nullptr);
            disconnect(&NetworkManager::instance(), &NetworkManager::sig_joinSuccessReceived, this, nullptr);
            disconnect(&NetworkManager::instance(), &NetworkManager::sig_playerReadyReceived, this, nullptr);
            disconnect(&NetworkManager::instance(), &NetworkManager::sig_gameStartReceived, this, nullptr);

            // 重置房间UI状态
            ui->lblRoomId->setText("Null");
            ui->btnStartGame->setStyleSheet("background-color: #4A6CF7;");
            setReady(false); // 重置准备状态

            // 重置全局变量
            OnlineSessionManager::instance()->setCurrentRoomId("");
            OnlineSessionManager::instance()->setMyOnlineColor("");

            // 跳回在线选择页面
            PageManager::instance()->switchToPage(2);
        }
        else
        {
            ui->lblRoomStatus->setText("等待对手加入...");
            ui->lblOpponentId->setText("未加入");
            ui->lblOpponentStatus->setText("未在线");
            ui->lblOpponentStatus->setText("未准备");
            ui->lblOpponentStatus->setStyleSheet("color: #FF6B6B");
            ui->btnStartGame->setEnabled(false);
        }
        });

    //监听玩家准备状态变更
    connect(&NetworkManager::instance(), &NetworkManager::sig_playerReadyReceived, this, [this](bool currentState,QString msg){
        if(currentState)
        {
            qDebug()<<OnlineSessionManager::instance()->getMyOnlineColor();
            if(OnlineSessionManager::instance()->getMyOnlineColor() == "BLACK")
            {
                ui->btnStartGame->setEnabled(true);
            }
            ui->lblOpponentStatus->setText("已准备");
            ui->lblOpponentStatus->setStyleSheet("color: #4A6CF7");
        }
        else
        {
            if(OnlineSessionManager::instance()->getMyOnlineColor() == "BLACK")
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
        ChessType myChessType = (OnlineSessionManager::instance()->getMyOnlineColor() == "BLACK") ? ChessType::BLACK : ChessType::WHITE;
        GameSession::instance()->setOnlinePlayerTag(myChessType, OnlineSessionManager::instance()->getMyOnlineColor());
        PageManager::instance()->switchToPage(5);
    });
}

OnlineRoomWidget::~OnlineRoomWidget()
{
    delete ui;
}

void OnlineRoomWidget::showEvent(QShowEvent *event)
{
    qDebug()<<OnlineSessionManager::instance()->getMyOnlineColor();
    QWidget::showEvent(event);
    ui->lblRoomId->setText(OnlineSessionManager::instance()->getCurrentRoomId().trimmed().toLower());
    ui->lblHostChess->setText("棋子：黑色");
    ui->lblOpponentChess->setText("棋子：白色");

    if (OnlineSessionManager::instance()->getMyOnlineColor() == "BLACK") {
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
    // 发送退出房间请求到服务端
    NetworkManager::instance().sendQuitRoomRequest();

}


void OnlineRoomWidget::on_btnCopyRoomId_clicked()
{
    QString roomId = ui->lblRoomId->text();
    QApplication::clipboard()->setText(roomId);
    // QMessageBox::information(this, "提示", "房间ID已复制到剪贴板！");
}


void OnlineRoomWidget::on_btnStartGame_clicked()
{
    if (OnlineSessionManager::instance()->getMyOnlineColor() == "BLACK") {
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
