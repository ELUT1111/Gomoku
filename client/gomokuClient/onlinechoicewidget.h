#ifndef ONLINECHOICEWIDGET_H
#define ONLINECHOICEWIDGET_H

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

signals:
    void sig_opponentChanged(bool status);
private:
    Ui::OnlineChoiceWidget *ui;
};

#endif // ONLINECHOICEWIDGET_H
