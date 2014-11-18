#ifndef CHOOSEDIALOG_H
#define CHOOSEDIALOG_H

#include <QDialog>
#include "appinfo.h"
#include "task.h"

namespace Ui {
    class ChooseDialog;
}

class ChooseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseDialog(AppInfo *appinfo,QWidget *parent = 0);
    ~ChooseDialog();
    Task *_exec();

private slots:
    void on_btOk_clicked();

    void on_btCancel_clicked();

private:
    AppInfo *m_info;
    Ui::ChooseDialog *ui;
    void closeEvent(QCloseEvent *evt);
};

#endif // CHOOSEDIALOG_H
