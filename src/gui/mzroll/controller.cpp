#include "controller.h"
#include "mainwindow.h"
#include <map>

#include <cstdlib>

Controller::Controller()
{
    mw = new MainWindow();
    updateUi();
    connect(mw->peakDetectionDialog, &PeakDetectionDialog::updateSettings, this, &Controller::updatePeakDetectionSettings);
    connect(mw->settingsForm, &SettingsForm::updateSettings,this,&Controller::updateOptionsDialogSettings);
}


template <typename T>
void Controller::syncMpWithUi(T* dialogPtr)
{


    QMap<QString, QVariant>& settings = dialogPtr->getSettings();

    for(const QString& key: settings.keys()) {

        const QVariant& v = settings.value(key);

        /** NOTE: QVariant is storing different  types and hence we can't say which type it points to. Therefore we check the typename to know what type 
        it's containing and then convert to it. can't say if this can be considered as a HACK or not **/

        if(QString(v.typeName()).contains("QDoubleSpinBox")) {
            qDebug() << v.value<QDoubleSpinBox*>()->value();
            updateMavenParameters(key, v.value<QDoubleSpinBox*>()->value());
        }

        if(QString(v.typeName()).contains("QGroupBox")) {
            updateMavenParameters(key, v.value<QGroupBox*>()->isChecked());
        }

        if(QString(v.typeName()).contains("QCheckBox")) {
            updateMavenParameters(key, v.value<QCheckBox*>()->isChecked());
        }

        if(QString(v.typeName()).contains("QSpinBox")) {
            updateMavenParameters(key, v.value<QSpinBox*>()->value());
        }

        if(QString(v.typeName()).contains("QSlider")) {
            updateMavenParameters(key, v.value<QSlider*>()->value());
        }

        if(QString(v.typeName()).contains("QComboBox")) {
            updateMavenParameters(key, v.value<QComboBox*>()->currentText());
        }

    }

}

void Controller::updateOptionsDialogSettings(OptionsDialogSettings* od)
{
    syncMpWithUi(od);
}

void Controller::updatePeakDetectionSettings(PeakDetectionSettings* pd)
{
    syncMpWithUi(pd);

}

void Controller::updateUi()
{
    std::map<std::string, std::string>& mavenSettings = mw->mavenParameters->getSettings();

    for(std::map<std::string, std::string>::iterator  it = mavenSettings.begin(); it != mavenSettings.end(); it++) {
        emit mw->peakDetectionDialog->settingsChanged(it->first, it->second);
        emit mw->settingsForm->settingsChanged(it->first, it->second);
    }
}

void Controller::updateMavenParameters(const QString& key,  const QVariant& value)
{

    /*TODO: can this be solved in a better way?.
     * In case of bool(if QVariant holds a bool val), "value.toByteArray().data()" retruns "false" for 0 and "true" for 1  but "false"  and "true" cant be converted 
     * to float(@see MavenParameters::setPeakDetectionSettings) and double(@see  PeakDetectionSettings::updatePeakSettings), hence we explicitly convert them 
     * to 0 and 1.
    */
    if(value.type() == QVariant::Bool) {

        bool val = value.toBool();
        if(val) {
            mw->mavenParameters->setPeakDetectionSettings(key.toLocal8Bit().data(),"1");
            mw->mavenParameters->setOptionsDialogSettings(key.toLocal8Bit().data(),"1");
        }

        else {
            mw->mavenParameters->setPeakDetectionSettings(key.toLocal8Bit().data(),"0");
            mw->mavenParameters->setOptionsDialogSettings(key.toLocal8Bit().data(),"0");
        }


        return;
    }

    mw->mavenParameters->setPeakDetectionSettings(key.toLocal8Bit().data(),value.toByteArray().data());
    mw->mavenParameters->setOptionsDialogSettings(key.toLocal8Bit().data(),value.toByteArray().data());
}