#include <QCloseEvent>
#include <QMessageBox>
#include "choosedialog.h"
#include "ui_choosedialog.h"
#include "appinfo_t.h"
#include "settingsdialog.h"

ChooseDialog::ChooseDialog(appinfo_t *info,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseDialog)
{
    ui->setupUi(this);
    ui->checkInstall->setChecked(true);
    m_info = info;

    if( info->isFlagSet(appinfo_t::ONLY_SILENT) )
    {
        ui->checkSilent->setChecked(true);;
        ui->checkSilent->setDisabled(true);
    }

    if(info->install_param.isEmpty())
        ui->checkSilent->setDisabled(true);
    else
    {
        if(SettingsDialog::getInstallMode()==SettingsDialog::SILENT)
            ui->checkSilent->setChecked(true);
    }

    for(int i=0;i<info->inet_files.count();i++)
    {
        ui->listWidget->addItem(info->inet_files[i].description);
    }

    ui->listWidget->setCurrentRow(0);
}

ChooseDialog::~ChooseDialog()
{
    delete ui;
}

void ChooseDialog::on_btOk_clicked()
{
    setResult(1);
    close();
}

void ChooseDialog::closeEvent(QCloseEvent *evt)
{
    if( evt->spontaneous() )
        setResult(0);
    evt->accept();
}

void ChooseDialog::on_btCancel_clicked()
{
    setResult(0);
    close();
}

task_t *ChooseDialog::exec()
{
    int result = QDialog::exec();
    if( result==0 )
        return NULL;

    int index = ui->listWidget->currentRow();

    int flags;
    flags = task_t::DOWNLOAD;
    if(ui->checkInstall->isChecked())
        flags |= task_t::INSTALL;
    if(ui->checkSilent->isChecked())
        flags |= task_t::SILENT;

    task_t *task = new task_t(m_info,
                              m_info->inet_files[index],
                              (task_t::task_flags)flags
                              );
    return task;
}
