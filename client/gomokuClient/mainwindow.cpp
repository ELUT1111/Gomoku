#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startwidget.h"
#include "offlinechoicewidget.h"
#include "gamewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initPage();
    connect(PageManager::instance(),&PageManager::signal_switchToPage,this,&MainWindow::slot_switchToPage);

    connect(this,&MainWindow::signal_undo,qobject_cast<GameWidget*>(gameWidget),&GameWidget::slot_undo);
    connect(this,&MainWindow::signal_reset,qobject_cast<GameWidget*>(gameWidget),&GameWidget::slot_reset);

}
void MainWindow::initPage()
{
    startWidget = new StartWidget();
    offlineChoiceWidget = new OfflineChoiceWidget();
    gameWidget = new GameWidget();
    ui->startLayout->addWidget(startWidget);
    ui->offlineChoiceLayout->addWidget(offlineChoiceWidget);

    ui->gameLayout_main->addWidget(gameWidget);

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

