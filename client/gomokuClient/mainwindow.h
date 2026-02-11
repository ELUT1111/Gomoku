#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <pagemanager.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:

    QWidget* startWidget;
    QWidget* offlineChoiceWidget;
    QWidget* onlineChoiceWidget;
    QWidget* onlineRoomWidget;
    QWidget* gameWidget;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initPage();
    inline void changePage(int index);

private:
    Ui::MainWindow *ui;

public slots:
    void slot_switchToPage(int index);

};

#endif // MAINWINDOW_H
