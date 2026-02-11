#include "startwidget.h"
#include "ui_startwidget.h"
#include "pagemanager.h"

StartWidget::StartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StartWidget)
{
    ui->setupUi(this);
}

StartWidget::~StartWidget()
{
    delete ui;
}

void StartWidget::on_offlineButton_clicked()
{
    PageManager::instance()->switchToPage(1);
}

void StartWidget::on_quitButton_clicked()
{
    QApplication::quit();
}

