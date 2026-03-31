#ifndef ONLINECHOICEWIDGET_H
#define ONLINECHOICEWIDGET_H

#include <QProgressDialog>
#include <QWidget>

namespace Ui {
class OnlineChoiceWidget;
}

class OnlineChoiceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineChoiceWidget(QWidget *parent = nullptr);
    ~OnlineChoiceWidget();

private slots:
    void on_returnButton_2_clicked();

    void on_addRoomButton_clicked();

    void on_JoinRoomButton_clicked();

    void on_searchGameButton_clicked();
    void on_roomListButton_clicked();

    void on_btnReconnect_clicked();

    void hideToast();

    // 服务器连接状态
    void onServerConnected();
    void onServerError(QString msg);

signals:
    void sig_opponentChanged(bool status);
private:
    bool m_isReconnecting = false;    // 重连中标记
    QString m_reconnectUrl;           // 重连地址
    // 初始化连接悬浮提示框
    void initToast();
    // 显示连接提示信息
    void showToast(const QString& text, bool isSuccess = true);

    Ui::OnlineChoiceWidget *ui;
    QProgressDialog* matchDialog = nullptr; // 匹配动画弹窗指针

    QLabel* m_toastLabel;       // 底部悬浮提示
    QTimer* m_toastTimer;       // 自动隐藏定时器
    QString m_btnOriginalText;  // 按钮原始文字
    QString m_btnOriginalStyle; // 按钮原始样式
};

#endif // ONLINECHOICEWIDGET_H
