#include "mainwindow.h"
#include "onlineroomwidget.h"
#include "ui_mainwindow.h"
#include "startwidget.h"
#include "offlinechoicewidget.h"
#include "gamewidget.h"

#include <OnlineChoiceWidget.h>
#include <OnlineRoomListWidget.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initPage();
    connect(PageManager::instance(),&PageManager::signal_switchToPage,this,&MainWindow::slot_switchToPage);

    connect(this,&MainWindow::signal_undo,qobject_cast<GameWidget*>(gameWidget),&GameWidget::slot_undo);
    connect(this,&MainWindow::signal_reset,qobject_cast<GameWidget*>(gameWidget),&GameWidget::slot_reset);
    connect(PageManager::instance(), &PageManager::signal_changeGamemode,
            this, &MainWindow::slot_updateGameButtonVisible);

}
void MainWindow::initPage()
{
    startWidget = new StartWidget();
    offlineChoiceWidget = new OfflineChoiceWidget();
    gameWidget = new GameWidget();
    onlineChoiceWidget = new OnlineChoiceWidget();
    onlineRoomWidget = new OnlineRoomWidget();
    onlineRoomListWidget = new OnlineRoomListWidget();

    ui->startLayout->addWidget(startWidget);
    ui->offlineChoiceLayout->addWidget(offlineChoiceWidget);
    ui->onlineChoiceLayout->addWidget(onlineChoiceWidget);
    ui->onlineRoomLayout->addWidget(onlineRoomWidget);

    ui->gameLayout_main->addWidget(gameWidget);
    ui->onlineRoomListLayout->addWidget(onlineRoomListWidget);

    changePage(0);
}
inline void MainWindow::changePage(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}
void MainWindow::slot_switchToPage(int index)
{
    changePage(index);
}

void MainWindow::slot_updateGameButtonVisible(GamemodeType mode)
{
    // 在线模式下，关闭重置和返回按钮
    bool isOnlineMode = (mode == GamemodeType::ONLINE);
    // ui->undoButton->setVisible(!isOnlineMode);
    ui->resetButton->setVisible(!isOnlineMode);
    ui->backButton->setVisible(!isOnlineMode);
}
MainWindow::~MainWindow()
{
    PageManager::instance()->destroyInstance();
    delete ui;
}

void MainWindow::on_undoButton_clicked()
{
    emit signal_undo();
}


void MainWindow::on_resetButton_clicked()
{
    emit signal_reset();
}


void MainWindow::on_backButton_clicked()
{
    PageManager::instance()->switchToPage(1);
    emit signal_reset();
}

