#include "onlinechoicewidget.h"
#include "ui_onlinechoicewidget.h"
#include "pagemanager.h"
#include "NetworkManager.h"
#include <OnlineSessionManager.h>
#include <QMessageBox>

OnlineChoiceWidget::OnlineChoiceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineChoiceWidget)
    , matchDialog(nullptr)
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

    // 绑定房间信息信号→跳转到房间页面
    QSharedPointer<QMetaObject::Connection> conn = QSharedPointer<QMetaObject::Connection>::create();
    *conn = connect(&NetworkManager::instance(), &NetworkManager::sig_createRoomStatusReceived, this,
                    [this,conn](QString roomId, QString player,bool status, QString msg){
                        Q_UNUSED(msg);

                        if (!this->isVisible()) return;

                        if (!status) return;

                        qDebug() << "[OnlineChoice] 创建房间成功，ID：" << roomId;

                        // 传递房间ID给房间页面
                        OnlineSessionManager::instance()->setCurrentRoomId(roomId.trimmed().toLower());

                        OnlineSessionManager::instance()->setMyOnlineColor("BLACK");

                        if(matchDialog) {
                            disconnect(matchDialog, &QProgressDialog::canceled, nullptr, nullptr);
                            matchDialog->close();
                            matchDialog->deleteLater();
                            matchDialog = nullptr;
                        }

                        if(!player.isEmpty()) {
                            OnlineSessionManager::instance()->setCurrentRoomId(roomId.trimmed().toLower());
                            OnlineSessionManager::instance()->setMyOnlineColor(player);
                            PageManager::instance()->switchToPage(3); // 跳转到房间页
                        }
                    });

    //绑定房间信息信号→跳转到房间页面
    QSharedPointer<QMetaObject::Connection> conn2 = QSharedPointer<QMetaObject::Connection>::create();
    *conn2 = connect(&NetworkManager::instance(), &NetworkManager::sig_joinSuccessReceived, this,
                    [this,conn2](QString roomId, QString player, QString msg){
                        Q_UNUSED(msg);
                        if (!this->isVisible()) return;
                        if(!player.isEmpty()) {
                            if(matchDialog) {
                                disconnect(matchDialog, &QProgressDialog::canceled, nullptr, nullptr);
                                matchDialog->close();
                                matchDialog->deleteLater();
                                matchDialog = nullptr;
                            }
                            OnlineSessionManager::instance()->setCurrentRoomId(roomId.trimmed().toLower());
                            OnlineSessionManager::instance()->setMyOnlineColor(player);
                            PageManager::instance()->switchToPage(3);
                        }
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

}
void OnlineChoiceWidget::on_searchGameButton_clicked()
{
    NetworkManager::instance().sendRandomMatchRequest();

    // 创建无限滚动的等候弹窗
    matchDialog = new QProgressDialog("正在寻找势均力敌的对手...", "取消匹配", 0, 0, this);
    matchDialog->setWindowTitle("随机匹配中");
    matchDialog->setWindowModality(Qt::WindowModal);
    matchDialog->setMinimumDuration(0); // 立即显示

    // 点击取消则发送取消请求，并销毁弹窗
    connect(matchDialog, &QProgressDialog::canceled, this, [this](){
        NetworkManager::instance().sendCancelMatchRequest();
        if(matchDialog) {
            matchDialog->deleteLater();
            matchDialog = nullptr;
        }
    });

    matchDialog->show();
}


void OnlineChoiceWidget::on_roomListButton_clicked()
{
    PageManager::instance()->switchToPage(4); // 4=房间列表
}

