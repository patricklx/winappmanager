#ifndef CHOOSEDIALOG_H
#define CHOOSEDIALOG_H

#include <QDialog>
#include "appinfo_t.h"
#include "task_t.h"

namespace Ui {
    class ChooseDialog;
}

class ChooseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseDialog(appinfo_t *appinfo,QWidget *parent = 0);
    ~ChooseDialog();
    task_t *exec();

private slots:
    void on_btOk_clicked();

    void on_btCancel_clicked();

private:
    appinfo_t *m_info;
    Ui::ChooseDialog *ui;
    void closeEvent(QCloseEvent *evt);
};

#endif // CHOOSEDIALOG_H
