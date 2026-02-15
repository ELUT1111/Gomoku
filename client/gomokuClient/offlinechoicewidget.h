#ifndef OFFLINECHOICEWIDGET_H
#define OFFLINECHOICEWIDGET_H

#include <QWidget>

namespace Ui {
class OfflineChoiceWidget;
}

class OfflineChoiceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OfflineChoiceWidget(QWidget *parent = nullptr);
    ~OfflineChoiceWidget();

private slots:
    void on_returnButton_clicked();

    void on_freeButton_clicked();

    void on_easyButton_clicked();

private:
    Ui::OfflineChoiceWidget *ui;
};

#endif // OFFLINECHOICEWIDGET_H
