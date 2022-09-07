#include "tachometer_window.h"
#include "ui_tachometer_window.h"

#include <QOpenGLWidget>

static uint32_t old_mileage = -1;



tachometer_window::tachometer_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tachometer_window)
{
    ui->setupUi(this);
    tacho_handle = new tachometer(PSA_TACHO_7K, this);
    tacho_handle->set_scale(0.5);
//    van_handle = new psa_van_receiver();
//    van_handle->receive_engine_packets();

//    QObject::connect(this->van_handle, &psa_van_receiver::engine_data_changed, this, &tachometer_window::receive_new_data);


    scene = tacho_handle->get_scene();

    speed_x = 0;
    speed_y = 0;


    widget = new QOpenGLWidget();

    //widget->setAttribute(Qt::WA_AlwaysStackOnTop);

    ui->tachometer->setViewport(widget);
    ui->tachometer->setBackgroundBrush(QPixmap(":/backgrounds/backgrounds/bg.png"));
    //ui->tachometer->setRenderHint(QPainter::Antialiasing);
    ui->tachometer->setScene(scene);
    ui->tachometer->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    ui->oil_temp_icon->hide();
    ui->oil_temp_label->hide();

    //ui->tachometer->setBackgroundBrush(QBrush(Qt::transparent));
    //ui->tachometer->setForegroundBrush(QBrush(Qt::transparent));
}

void tachometer_window::receive_engine_data(psa_engine_data_t engine_data)
{
    if(engine_data.speed != 0xffff && engine_data.rpm != 0xffff)
    {
        float speed = (float)engine_data.speed / 100;
        int rpm = (float)engine_data.rpm / 8;
        change_speed(speed);
        change_tacho_angle(engine_data.rpm / 8);
        if(engine_data.reverse)
        {
            tacho_handle->set_gear(GEAR_R);
        }
        else if(rpm > 1100)
        {
            // Hard coded gear ratios for my car
            float gear_ratio = (speed / rpm) * 100;

            if(gear_ratio > 0.65f && gear_ratio < 0.85f)
            {
                tacho_handle->set_gear(GEAR_1);
            }
            else if(gear_ratio > 1.35f && gear_ratio < 1.55f)
            {
                tacho_handle->set_gear(GEAR_2);
            }
            else if(gear_ratio > 1.95f && gear_ratio < 2.15f)
            {
                tacho_handle->set_gear(GEAR_3);
            }
            else if(gear_ratio > 2.50f && gear_ratio < 2.85f)
            {
                tacho_handle->set_gear(GEAR_4);
            }
            else if(gear_ratio > 3.20f && gear_ratio < 3.60f)
            {
                tacho_handle->set_gear(GEAR_5);
            }
            else{
                tacho_handle->set_gear(GEAR_N);
            }
        }
        else{
            tacho_handle->set_gear(GEAR_N);
        }
    }
    else{
        tacho_handle->set_gear(GEAR_N);
    }

    if(engine_data.engine_temperature != 0xff)
    {
        ui->temp_label->setText(QString::asprintf("%3d°C", engine_data.engine_temperature-40));
    }

    if(engine_data.oil_temperature != 0 && engine_data.oil_temperature != 0xff)
    {
        ui->oil_temp_icon->show();
        ui->oil_temp_label->show();
        ui->oil_temp_label->setText(QString::asprintf("%3d°C", engine_data.oil_temperature - 40));
    }else{
        ui->oil_temp_icon->hide();
        ui->oil_temp_label->hide();
    }

    ui->fuel_label->setText(QString::asprintf("%2d%%", engine_data.fuel_level));
}

void tachometer_window::receive_dash_data(psa_dash_data_t dash_data)
{
    if(dash_data.mileage != old_mileage)
    {
        this->ui->mileage_label->setText(QString::asprintf("%8.1f", (float)dash_data.mileage / 10.f));
        old_mileage = dash_data.mileage;
    }
}

void tachometer_window::change_tacho_angle(int rpm)
{
    //ui->rpm_label->setText(QString::asprintf("RPM: %4d", rpm));
    //ui->y_label->setText(QString::asprintf("X: %3d", rpm));
    tacho_handle->set_rpm(rpm);
//    speed_y = rpm;
//    tacho_handle->set_speed_pos(speed_x, speed_y);
}

void tachometer_window::change_speed(int speed)
{
    //ui->x_label->setText(QString::asprintf("X: %3d", speed));
    tacho_handle->set_speed(speed);
//    speed_x = speed;
//    tacho_handle->set_speed_pos(speed_x, speed_y);
}

void tachometer_window::update_display()
{
    tacho_handle->update_display();
}

tachometer_window::~tachometer_window()
{
    ui->tachometer->close();
    //delete van_handle;
    //delete tacho_handle;
    delete ui;
}
