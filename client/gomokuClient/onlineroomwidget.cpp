#include "onlineroomwidget.h"
#include "ui_onlineroomwidget.h"

#include <PageManager.h>

OnlineRoomWidget::OnlineRoomWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnlineRoomWidget)
{
    ui->setupUi(this);
}

OnlineRoomWidget::~OnlineRoomWidget()
{
    delete ui;
}

void OnlineRoomWidget::on_btnQuitRoom_clicked()
{
    PageManager::instance()->switchToPage(2);
}

