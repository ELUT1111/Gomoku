#ifndef ROOMITEMWIDGET_H
#define ROOMITEMWIDGET_H

#include <QWidget>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class RoomItemWidget; }
QT_END_NAMESPACE

class RoomItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RoomItemWidget(QWidget *parent = nullptr);
    ~RoomItemWidget();

    // 设置房间数据
    void setRoomData(const QString& roomId, const QString& status, int playerCount);
    // 获取房间ID
    QString getRoomId() const { return m_roomId; }

signals:
    // 点击加入信号
    void sigJoinRoom(QString roomId);

private slots:
    void on_btnJoin_clicked();

private:
    Ui::RoomItemWidget *ui;
    QString m_roomId;
};

#endif // ROOMITEMWIDGET_H