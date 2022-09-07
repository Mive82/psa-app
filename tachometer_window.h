#ifndef TACHOMETER_WINDOW_H
#define TACHOMETER_WINDOW_H

#include <QWidget>
#include <QOpenGLWidget>
#include "tachometer.h"
#include "psa_van_receiver.h"

namespace Ui {
class tachometer_window;
}

class tachometer_window : public QWidget
{
    Q_OBJECT

public:
    explicit tachometer_window(QWidget *parent = nullptr);
    ~tachometer_window();

    QTimer* screen_refresh_timer;
    void change_tacho_angle(int rpm);
    void update_display();
    void change_speed(int speed);

public slots:
    void receive_engine_data(psa_engine_data_t engine_data);
    void receive_dash_data(psa_dash_data_t dash_data);
private:
    Ui::tachometer_window *ui;

    QGraphicsScene *scene;
    tachometer *tacho_handle;
    psa_van_receiver *van_handle;
    QOpenGLWidget* widget;
    int speed_x, speed_y;

};

#endif // TACHOMETER_WINDOW_H
