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

