#include "onlinechoicewidget.h"
#include "ui_onlinechoicewidget.h"
#include "pagemanager.h"

OnlineChoiceWidget::OnlineChoiceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineChoiceWidget)
{
    ui->setupUi(this);
}

OnlineChoiceWidget::~OnlineChoiceWidget()
{
    delete ui;
}

void OnlineChoiceWidget::on_returnButton_2_clicked()
{
    PageManager::instance()->switchToPage(0);
}


void OnlineChoiceWidget::on_addRoomButton_clicked()
{
    PageManager::instance()->switchToPage(3);
}

