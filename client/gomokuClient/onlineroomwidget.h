#ifndef ONLINEROOMWIDGET_H
#define ONLINEROOMWIDGET_H

#include <QWidget>

namespace Ui {
class OnlineRoomWidget;
}

class OnlineRoomWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineRoomWidget(QWidget *parent = nullptr);
    ~OnlineRoomWidget();

private slots:
    void on_btnQuitRoom_clicked();

private:
    Ui::OnlineRoomWidget *ui;
};

#endif // ONLINEROOMWIDGET_H
