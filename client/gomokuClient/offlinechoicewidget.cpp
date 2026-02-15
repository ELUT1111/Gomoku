#include "offlinechoicewidget.h"
#include "ui_offlinechoicewidget.h"
#include "pagemanager.h"

OfflineChoiceWidget::OfflineChoiceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OfflineChoiceWidget)
{
    ui->setupUi(this);
}

OfflineChoiceWidget::~OfflineChoiceWidget()
{
    delete ui;
}

void OfflineChoiceWidget::on_returnButton_clicked()
{
    PageManager::instance()->switchToPage(0);
}


void OfflineChoiceWidget::on_freeButton_clicked()
{
    PageManager::instance()->switchToPage(4);
    PageManager::instance()->signal_changeGamemode(GamemodeType::OFFLINE_FREE);
}


void OfflineChoiceWidget::on_easyButton_clicked()
{
    PageManager::instance()->switchToPage(4);
    PageManager::instance()->signal_changeGamemode(GamemodeType::OFFLINE_AI);
}

