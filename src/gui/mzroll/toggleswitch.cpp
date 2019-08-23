#include "toggleswitch.h"

ToggleSwitch::ToggleSwitch(QWidget *parent)
    : QSlider(Qt::Orientation::Horizontal, parent)
{
    _switchState = SwitchState::Off;
    setMouseTracking(true);
    setMaximumWidth(32);
    setMinimum(0);
    setMaximum(maximumWidth() / 2);
    setPageStep(1);
    setStyleSheet(_passiveStyle());
}

void ToggleSwitch::toggleSwitch()
{
    if (_switchState == SwitchState::Off) {
        _switchState = SwitchState::On;
        for (int i = 0; i <= maximum(); ++i) {
            QThread::msleep(1);
            QApplication::processEvents();
            setValue(i);
        }
    } else {
        _switchState = SwitchState::Off;
        for (int i = maximum(); i >= 0; --i) {
            QThread::msleep(1);
            QApplication::processEvents();
            setValue(i);
        }
    }
}

QString ToggleSwitch::_passiveStyle()
{
    QString style("QSlider::groove:horizontal { "
                  "background: white; "
                  "height: 12px; "
                  "border-radius: 4px; "
                  "} "

                  "QSlider::sub-page:horizontal { "
                  "background: #ccf; "
                  "border: 1px solid #777; "
                  "height: 12px; "
                  "border-radius: 4px; "
                  "} "

                  "QSlider::add-page:horizontal { "
                  "background: #fff; "
                  "border: 1px solid #777; "
                  "height: 12px; "
                  "border-radius: 4px; "
                  "} "

                  "QSlider::handle:horizontal { "
                  "background: qlineargradient(x1:0.5, y1:0, x2:0.5, y2:1,"
                  "                            stop:0 #eee, stop:1 #ccc); "
                  "border: 1px solid #777; "
                  "width: 15px; "
                  "margin-top: -2px; "
                  "margin-bottom: -2px; "
                  "border-radius: 4px; "
                  "} "

                  "QSlider::sub-page:horizontal:disabled { "
                  "background: #bbb; "
                  "border-color: #aaa; "
                  "} "

                  "QSlider::add-page:horizontal:disabled { "
                  "background: #eee; "
                  "border-color: #aaa; "
                  "} "

                  "QSlider::handle:horizontal:disabled { "
                  "background: #eee; "
                  "border: 1px solid #999; "
                  "border-radius: 4px; "
                  "}");
    return style;
}

QString ToggleSwitch::_hoverStyle()
{
    QString hoverStyle("QSlider::handle:horizontal { "
                       "background: qlineargradient(x1:0.5, y1:0, x2:0.5, y2:1,"
                       "                            stop:0 #fff, stop:1 #ddd); "
                       "border: 1px solid #777; "
                       "margin-top: -2px; "
                       "margin-bottom: -2px; "
                       "border-radius: 4px; "
                       "} ");
    return _passiveStyle() + hoverStyle;
}
