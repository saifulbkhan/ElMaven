#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QWidget>

class ToggleSwitch : public QSlider
{
    Q_OBJECT

public:
    enum class SwitchState { On, Off };

    ToggleSwitch(QWidget *parent = nullptr);
    ~ToggleSwitch() = default;
    void toggleSwitch();
    inline SwitchState switchState() { return _switchState; }

protected:
    void mousePressEvent(QMouseEvent *event) {}
    void mouseReleaseEvent(QMouseEvent *event) { toggleSwitch(); }
    void enterEvent(QEvent *event) { setStyleSheet(_hoverStyle()); }
    void leaveEvent(QEvent *event) { setStyleSheet(_passiveStyle()); }

private:
    SwitchState _switchState;
    QString _passiveStyle();
    QString _hoverStyle();
};

#endif // TOGGLESWITCH_H
