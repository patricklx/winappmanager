#include <QCloseEvent>
#include <QMessageBox>
#include <QInputDialog>
#include "choosedialog.h"
#include "ui_choosedialog.h"
#include "appinfo.h"
#include "settingsdialog.h"

ChooseDialog::ChooseDialog(AppInfo *info,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseDialog)
{
    ui->setupUi(this);
    ui->checkInstall->setChecked(true);
    m_info = info;
    this->setWindowTitle("Select: "+m_info->name());

    if( info->getInstallInfo().onlySilent )
    {
        ui->checkSilent->setChecked(true);
        ui->checkSilent->setDisabled(true);
    }

    if(info->getInstallInfo().silentParameter.isEmpty())
        ui->checkSilent->setDisabled(true);
    else
    {
        if(SettingsDialog::value<int>(SettingsDialog::InstallMode)==SettingsDialog::Silent)
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
        ui->listWidget->addItem(info->urls()[i].description);
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

Task *ChooseDialog::execute()
{
    int result = QDialog::exec();
    if( result==0 )
        return NULL;

    int index = ui->listWidget->currentRow();

    int flags;
    if( !m_info->isFlagSet(AppInfo::Downloaded) ||
            m_info->getUserData().downloadedVersion != m_info->getUserData().latestVersion ){

        flags = Task::DOWNLOAD;
    }

    if(ui->checkInstall->isChecked())
        flags |= Task::INSTALL;
    if(ui->checkSilent->isChecked())
        flags |= Task::SILENT;

    AppInfo::UrlContainer url = m_info->urls()[index];
    url.description="";
    qDebug()<<"parameter"<<url.parameter;

    QMapIterator<QString, QString> i(url.parameter);
    while (i.hasNext()) {
        i.next();
        qDebug()<<"choice: "<<i.key();
        QStringList choices = i.value().split(";");
        qDebug()<<"choices: "<<i.value();
        QStringList languages;
        if(i.key()=="lang"){

            foreach(QString choice,choices)
            {

                QLocale locale(choice);
                if(locale.language()!=QLocale::C){
                    QLocale::Language lang = locale.language();
                    QLocale::Country country = locale.country();
                    languages.append(QLocale::languageToString(lang)+
                                     " "+
                                     QLocale::countryToString(country));
                }
                else
                    languages.append(choice);
            }
            if(languages.count()>1){
                QString lang = QInputDialog::getItem(this,"Select "+i.key(),i.key()+": ",languages);
                int index = languages.indexOf(lang);
                url.parameter["lang"]=choices.at(index);
                m_info->setChoice(i.key(),choices.at(index));
                url.description += "lang: "+choices.at(index)+", ";
            }
        }else{

            if(choices.count()>1){
                QString choice = QInputDialog::getItem(this,"Select "+i.key(),i.key()+": ",choices);
                url.parameter[i.key()]=choice;
                m_info->setChoice(i.key(),choice);
                url.description += i.key()+": "+choice+", ";
            }
        }
        if( choices.count()==1 )
            m_info->setChoice(i.key(),i.value());
    }

    Task *task = new Task(m_info,
                             url,
                              (Task::task_flags)flags
                              );
    return task;
}
