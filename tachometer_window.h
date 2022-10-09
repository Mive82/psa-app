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
    void set_van_handle(psa_van_receiver* van_handle);

public slots:
    void receive_engine_data(psa_engine_data_t engine_data);
    void receive_dash_data(psa_dash_data_t dash_data);
    void receive_trip_data(psa_trip_data_t trip_data);
    void toggle_trip_visibility();


    void set_rpm_text_debug(int rpm);

    void send_trip_reset_a();
    void send_trip_reset_b();

private:
    Ui::tachometer_window *ui;

    QGraphicsScene *scene;
    tachometer *tacho_handle;
    psa_van_receiver *van_handle;
    QOpenGLWidget* widget;
    int speed_x, speed_y;
    psa_van_receiver *van_receiver = NULL;

    int window_visible = 0;

};

#endif // TACHOMETER_WINDOW_H
