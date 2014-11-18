#include <QCloseEvent>
#include <QMessageBox>
#include <QInputDialog>
#include "choosedialog.h"
#include "ui_choosedialog.h"
#include "appinfo.h"
#include "settingsdialog.h"

ChooseDialog::ChooseDialog(AppInfo *info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseDialog)
{
    ui->setupUi(this);
    ui->checkInstall->setChecked(true);
    m_info = info;
    this->setWindowTitle("Select: "+m_info->name());

    if( info->isFlagSet(AppInfo::OnlySilent) )
    {
        ui->checkSilent->setChecked(true);;
        ui->checkSilent->setDisabled(true);
    }

    if(info->getInstallInfo().silentParameter.isEmpty())
        ui->checkSilent->setDisabled(true);
    else
    {
        if(SettingsDialog::value("INSTALL_MODE").toInt()==SettingsDialog::Silent)
            ui->checkSilent->setChecked(true);
    }

    if(info->isFlagSet(AppInfo::NoRegistry) || !info->isFlagSet(AppInfo::WinVersionOk))
    {
        ui->checkInstall->setChecked(false);
        ui->checkInstall->setDisabled(true);
        ui->checkSilent->setDisabled(true);
        ui->checkSilent->setChecked(false);
    }

    for(int i=0;i<info->urls().count();i++)
    {
        ui->listWidget->addItem(info->urls().at(i).description);
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

Task *ChooseDialog::_exec()
{
    int result = QDialog::exec();
    if( result==0 )
        return NULL;

    int index = ui->listWidget->currentRow();
    AppInfo::UrlContainer url = m_info->urls().at(index);

    foreach(QString key, url.parameter.keys() ){

        forever{
            QString param = url.parameter[key];
            QStringList choices = param.split(";");
            if( key == "lang" ){
                QStringList tmp;
                foreach(QString lang, choices) {
                    QLocale locale(lang);
                    QLocale::Language l = locale.language();
                    QLocale::Country c = locale.country();
                    QString str = QLocale::languageToString(l) + " - " + QLocale::countryToString(c);
                    tmp.append(str);
                }
                choices = tmp;
            }
            if( choices.length()>0 ){
                QString choice = QInputDialog::getItem(this,"choose " + key,key, choices,0,false);
                int index = choices.indexOf(choice);
                choices = param.split(";");
                choice = choices.at(index);
                url.url.replace(QString("{%1}").arg(key),choice);
                break;
            }
        }
    }

    int flags;
    flags = Task::DOWNLOAD;
    if(ui->checkInstall->isChecked())
        flags |= Task::INSTALL;
    if(ui->checkSilent->isChecked())
        flags |= Task::SILENT;

    Task *task = new Task(m_info,
                              url,
                              (Task::task_flags)flags
                              );
    return task;
}
