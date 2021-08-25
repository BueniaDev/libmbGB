#ifndef MBGBQT_SETTINGS_H
#define MBGBQT_SETTINGS_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <libmbGB/libmbgb.h>
using namespace gb;
using namespace std;

namespace Ui
{
    class SettingsDialog;
}

class mbGBSettings : public QDialog
{
    Q_OBJECT

    public:
	mbGBSettings(GBCore *cb, QWidget *parent = NULL);
	~mbGBSettings();

	void initSelections();
	void initLinkDeviceIndex();
	void initMobileConfig();
	void updateMobileConfig();

	void readSettings(QSettings &settings);
	void writeSettings(QSettings &settings);

    public slots:
	void acceptChanges();
	void rejectChanges();
	void createMobileConf();
	void removeMobileConf();

    private:
	GBCore *core;

	int currentSystemType = 0;
	int currentLinkDevice = 0;
	QString currentMobileText = "";

	Ui::SettingsDialog *ui;
};

#endif // MBGBQT_SETTINGS_H