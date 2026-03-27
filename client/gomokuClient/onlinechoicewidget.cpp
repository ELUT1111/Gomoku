#include "onlinechoicewidget.h"
#include "ui_onlinechoicewidget.h"
#include "pagemanager.h"
#include "NetworkManager.h"
#include <OnlineSessionManager.h>
#include <QMessageBox>
#include <qtimer.h>

OnlineChoiceWidget::OnlineChoiceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineChoiceWidget)
    , matchDialog(nullptr)
{
    ui->setupUi(this);

    initToast();

    m_btnOriginalText = ui->btnReconnect->text();
    m_btnOriginalStyle = ui->btnReconnect->styleSheet();

    connect(&NetworkManager::instance(), &NetworkManager::connected, this, &OnlineChoiceWidget::onServerConnected);
    connect(&NetworkManager::instance(), &NetworkManager::errorOccurred, this, &OnlineChoiceWidget::onServerError);

    ui->lineEdit_server->setText(WS_SERVER_URL);

    // 绑定创建房间信息返回信号→跳转到房间页面
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

    //绑定加入房间信息返回信号→跳转到房间页面
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

    QTimer::singleShot(0, this, [this](){
        // 初次默认连接
        NetworkManager::instance().connectToServer(WS_SERVER_URL);
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


void OnlineChoiceWidget::on_btnReconnect_clicked()
{
    if(m_isReconnecting) return;
    // 获取输入的服务器地址，为空则使用默认地址
    QString serverUrl = ui->lineEdit_server->text().trimmed();
    if (serverUrl.isEmpty()) {
        serverUrl = WS_SERVER_URL;
        ui->lineEdit_server->setText(serverUrl);
    }
    m_reconnectUrl = serverUrl;

    ui->btnReconnect->setEnabled(false);
    ui->btnReconnect->setText("🔄 正在连接...");
    ui->btnReconnect->setStyleSheet(R"(
        QPushButton {
            background-color: #3498db;
            color: white;
            font-weight: bold;
            border-radius: 8px;
            border: none;
            font-size: 14px;
            height: 36px;
        }
    )");
    m_isReconnecting = true;
    connect(&NetworkManager::instance(), &NetworkManager::errorOccurred,
            this, &OnlineChoiceWidget::onReconnectAfterDisconnected,
            Qt::SingleShotConnection);
    connect(&NetworkManager::instance(), &NetworkManager::connected,
            this, &OnlineChoiceWidget::onReconnectFinished,
            Qt::SingleShotConnection);
    // 主动断开旧连接
    NetworkManager::instance().disconnectFromServer();
    // NetworkManager::instance().connectToServer(serverUrl);
}

void OnlineChoiceWidget::initToast()
{
    // 创建悬浮标签
    m_toastLabel = new QLabel(this);
    m_toastLabel->setAlignment(Qt::AlignCenter);
    m_toastLabel->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 14px;
            font-weight: bold;
            padding: 8px 20px;
            border-radius: 18px;
            background-color: rgba(46, 204, 113, 0.6);  /* 成功绿色 */
        }
    )");
    m_toastLabel->hide();  // 默认隐藏
    // m_toastLabel->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    m_toastLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 不响应鼠标事件

    // 自动隐藏定时器（2秒后消失）
    m_toastTimer = new QTimer(this);
    m_toastTimer->setSingleShot(true);
    connect(m_toastTimer, &QTimer::timeout, this, &OnlineChoiceWidget::hideToast);
}

void OnlineChoiceWidget::showToast(const QString &text, bool isSuccess)
{
    m_toastLabel->setText(text);
    // 成功=绿色，失败=红色
    if(isSuccess) {
        m_toastLabel->setStyleSheet("background-color: rgba(46, 204, 113, 0.6); color:white; font-size:14px; padding:8px 20px; border-radius:18px;");
    } else {
        m_toastLabel->setStyleSheet("background-color: rgba(255, 82, 82, 0.6); color:white; font-size:14px; padding:8px 20px; border-radius:18px;");
    }

    m_toastLabel->adjustSize();

    // 定位到界面底部居中
    int x = (this->width() - m_toastLabel->width()) / 2;
    int y = this->height() - 30;
    m_toastLabel->move(x, y);
    m_toastLabel->show();
    m_toastTimer->start(2000); // 2秒后自动隐藏
}

void OnlineChoiceWidget::hideToast()
{
    m_toastLabel->hide();
}

void OnlineChoiceWidget::onServerConnected()
{
    // 重置按钮为原始状态
    ui->btnReconnect->setEnabled(true);
    ui->btnReconnect->setText(m_btnOriginalText);
    ui->btnReconnect->setStyleSheet(m_btnOriginalStyle);

    // 显示成功悬浮提示
    showToast("✅ 服务器连接成功", true);
}

void OnlineChoiceWidget::onServerError(QString msg)
{
    // 重置按钮为原始状态
    ui->btnReconnect->setEnabled(true);
    ui->btnReconnect->setText(m_btnOriginalText);
    ui->btnReconnect->setStyleSheet(m_btnOriginalStyle);

    // 显示失败悬浮提示
    showToast("❌ 连接失败：" + msg, false);
}

void OnlineChoiceWidget::onReconnectAfterDisconnected()
{
    if(!m_isReconnecting) return;

    // 异步重连
    NetworkManager::instance().connectToServer(m_reconnectUrl);
}

void OnlineChoiceWidget::onReconnectFinished()
{
    m_isReconnecting = false;
}
