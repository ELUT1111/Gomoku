#include "roomitemwidget.h"
#include "ui_roomitemwidget.h"

RoomItemWidget::RoomItemWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RoomItemWidget)
{
    ui->setupUi(this);
    // 固定高度，保证列表项整齐
    this->setFixedHeight(80);
}

RoomItemWidget::~RoomItemWidget()
{
    delete ui;
}

void RoomItemWidget::setRoomData(const QString &roomId, const QString &status, int playerCount)
{
    m_roomId = roomId;
    ui->lblRoomId->setText("房间号：" + roomId);
    ui->lblPlayerCount->setText(QString("人数：%1/2").arg(playerCount));

    // 状态：胶囊背景色 + 文字，视觉极强
    if(status == "WAIT") {
        ui->lblStatus->setText("等待中");
        ui->lblStatus->setStyleSheet("background-color: #E8F3FF; color: #4A6CF7;");
        ui->btnJoin->setEnabled(playerCount < 2);
    } else if(status == "PLAYING") {
        ui->lblStatus->setText("游戏中");
        ui->lblStatus->setStyleSheet("background-color: #FFF4E8; color: #FF8C38;");
        ui->btnJoin->setEnabled(false);
    } else {
        ui->lblStatus->setText("已关闭");
        ui->lblStatus->setStyleSheet("background-color: #F5F7FA; color: #999999;");
        ui->btnJoin->setEnabled(false);
    }
}

void RoomItemWidget::on_btnJoin_clicked()
{
    emit sigJoinRoom(m_roomId);
}