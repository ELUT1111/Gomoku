#ifndef ONLINEROOMWIDGET_H
#define ONLINEROOMWIDGET_H

#include <QLabel>
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
    bool getReady() const;
    void setReady(bool newReady);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_btnQuitRoom_clicked();

    void on_btnCopyRoomId_clicked();

    void on_btnStartGame_clicked();

private:
    bool ready = false;

    Ui::OnlineRoomWidget *ui;
};

#endif // ONLINEROOMWIDGET_H
