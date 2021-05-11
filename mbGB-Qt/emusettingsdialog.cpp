#include "emusettingsdialog.h"
#include "ui_emusettingsdialog.h"
using namespace gb;
using namespace std;

mbGBSettings::mbGBSettings(GBCore *cb, QWidget *parent) : core(cb), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    resize(400, 300);
    setWindowTitle("Settings - mbGB-Qt");

    ui->systemCombo->addItem("Auto");
    ui->systemCombo->addItem("Game Boy [DMG]");
    ui->systemCombo->addItem("Game Boy Color [CGB]");
    ui->systemCombo->addItem("Game Boy Advance [AGB]");

    ui->linkCombo->addItem("None");
    ui->linkCombo->addItem("Game Boy Printer");
    ui->linkCombo->addItem("Mobile Adapter GB");
    ui->linkCombo->addItem("Barcode Boy");
    ui->linkCombo->addItem("Turbo File GB");

    connect(ui->confirmButtons, SIGNAL(accepted()), this, SLOT(acceptChanges()));
    connect(ui->confirmButtons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
}

mbGBSettings::~mbGBSettings()
{

}

void mbGBSettings::readSettings(QSettings &settings)
{

}

void mbGBSettings::writeSettings(QSettings &settings)
{

}

void mbGBSettings::initLinkDeviceIndex()
{
    
}

void mbGBSettings::initSelections()
{

}

void mbGBSettings::acceptChanges()
{
    core->paused = false;
    accept();
}

void mbGBSettings::rejectChanges()
{
    core->paused = false;
    reject();
}