#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startwidget.h"
#include "offlinechoicewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initPage();
    connect(PageManager::instance(),&PageManager::signal_switchToPage,this,&MainWindow::slot_switchToPage);

}
void MainWindow::initPage()
{
    startWidget = new StartWidget();
    offlineChoiceWidget = new OfflineChoiceWidget();
    ui->startLayout->addWidget(startWidget);
    ui->offlineChoiceLayout->addWidget(offlineChoiceWidget);
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
