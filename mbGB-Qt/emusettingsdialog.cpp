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

    connect(ui->mobileConfCreate, SIGNAL(clicked()), this, SLOT(createMobileConf()));
    connect(ui->mobileConfDelete, SIGNAL(clicked()), this, SLOT(removeMobileConf()));
}

mbGBSettings::~mbGBSettings()
{

}

void mbGBSettings::readSettings(QSettings &settings)
{
    currentSystemType = settings.value("systemType").toInt();
    currentLinkDevice = settings.value("linkDevice").toInt();
    int count = settings.value("mobileConfCount").toInt();

    if (count > 0)
    {
	for (int index = 0; index < count; index++)
	{
	    QString key = "mobileConfEntry";
	    key.append(QString::number(index));
	    QString textVal = settings.value(key, "").toString();
	    ui->mobileConfCombo->addItem(textVal);
	}
    }

    QString defaultText = ui->mobileConfCombo->itemText(0);
    currentMobileText = settings.value("mobileConfActive", defaultText).toString();

    initSelections();
}

void mbGBSettings::writeSettings(QSettings &settings)
{
    settings.setValue("systemType", currentSystemType);
    settings.setValue("linkDevice", currentLinkDevice);
    int count = ui->mobileConfCombo->count();
    settings.setValue("mobileConfCount", count);

    if (count > 0)
    {
	for (int index = 0; index < count; index++)
	{
	    QString key = "mobileConfEntry";
	    key.append(QString::number(index));
	    settings.setValue(key, ui->mobileConfCombo->itemText(index));
	}
    }

    settings.setValue("mobileConfActive", ui->mobileConfCombo->currentText());
}

void mbGBSettings::initLinkDeviceIndex()
{
    ui->linkCombo->setCurrentIndex(currentLinkDevice);
    core->connectserialdevice(currentLinkDevice);
}

void mbGBSettings::initMobileConfig()
{
    updateMobileConfig();
    core->setaddonfilename(currentMobileText.toStdString());
}

void mbGBSettings::updateMobileConfig()
{
    if (ui->mobileConfCombo->count() > 0)
    {
	ui->mobileConfDelete->setEnabled(true);
    }
    else
    {
	ui->mobileConfDelete->setEnabled(false);
    }
}

void mbGBSettings::initSelections()
{
    core->setsystemtype(currentSystemType);
    initLinkDeviceIndex();
    initMobileConfig();
}

void mbGBSettings::acceptChanges()
{
    int systemType = ui->systemCombo->currentIndex();
    int linkDevice = ui->linkCombo->currentIndex();
    QString mobileText = ui->mobileConfCombo->currentText();

    if (systemType != currentSystemType
	|| linkDevice != currentLinkDevice
	|| mobileText != currentMobileText) 
    {
	if (core->isrunning())
	{
	    auto result = QMessageBox::warning(this, "Reset necessary to apply changes", 
		"The emulation will be reset for these changes to take effect.", 
		QMessageBox::Ok | QMessageBox::Cancel);

	    if (result != QMessageBox::Ok)
	    {
		return;
	    }
	}

	currentSystemType = systemType;
	currentLinkDevice = linkDevice;
	currentMobileText = mobileText;

	initSelections();

	if (core->isrunning())
	{
	    core->resetcore();
	}
    }

    core->paused = false;
    accept();
}

void mbGBSettings::rejectChanges()
{
    core->paused = false;
    reject();
}

void mbGBSettings::createMobileConf()
{
    QString filename = QFileDialog::getSaveFileName(this, "Enter the name of the config file...",
			QDir::currentPath(), 
			"Mobile Adapter config files (*.mbconf)");

    if (filename.isEmpty()) return;

    if (ui->mobileConfCombo->findText(filename) >= 0)
    {
	QMessageBox::critical(this, "Error", "The selected config file already exists.");
        return;
    }

    ui->mobileConfCombo->addItem(filename);
}

void mbGBSettings::removeMobileConf()
{
    auto result = QMessageBox::question(this, "Confirm", 
	"Are you sure you want to get rid of the current config file?");

    if (result != QMessageBox::Yes)
    {
	return;
    }

    ui->mobileConfCombo->removeItem(ui->mobileConfCombo->currentIndex());

    currentMobileText = ui->mobileConfCombo->currentText();
    initMobileConfig();
}