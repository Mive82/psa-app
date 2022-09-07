#include "tachometer.h"
//#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QTimer>

#include <math.h>


#include <QtConcurrent/QtConcurrentRun>


tachometer::tachometer(QObject *parent) : QObject{parent}
{
    create_tachometer(PSA_TACHO_7K);
}

tachometer::tachometer(tacho_range_t range, QObject *parent) : QObject{parent}
{
    create_tachometer(range);
}

void tachometer::create_tachometer(tacho_range_t range)
{
    this->gear_timer = new QTimer(this);
    connect(this->gear_timer, &QTimer::timeout, this, &tachometer::update_gear_display);
    this->speed_update_watcher = new QFutureWatcher<void>(this);
    this->rpm_update_watcher = new QFutureWatcher<void>(this);

    QObject::connect(this->speed_update_watcher, &QFutureWatcher<void>::finished, this, &tachometer::update_display);
    QObject::connect(this->rpm_update_watcher, &QFutureWatcher<void>::finished, this, &tachometer::update_display);
    this->scene = new QGraphicsScene();

    tacho_bg = new QPixmap(":/tachometer/tacho/background.png");
    tacho_needle = new QPixmap(":/tachometer/tacho/needle.png");

    switch(range)
    {
    case PSA_TACHO_7K:
    default:
        tacho_numbers = new QPixmap(":/tachometer/tacho/7k.png");
        this->set_range = PSA_TACHO_7K;
        break;
    case PSA_TACHO_8K:
        tacho_numbers = new QPixmap(":/tachometer/tacho/8k.png");
        this->set_range = PSA_TACHO_8K;
        break;
    }


    tacho_img_width = tacho_bg->width();
    tacho_img_height = tacho_bg->height();

    int scene_w = 800;
    int scene_h = 400;

    int tacho_bg_offset_x = (scene_w - tacho_img_width) / 2;
    int tacho_bg_offset_y = (scene_h - tacho_img_height) / 2;

    scene->setSceneRect(0,0,scene_w, scene_h);

    tacho_needle_offset_x = (tacho_img_width - tacho_needle->width()) / 2;
    tacho_needle_offset_y = (tacho_img_height - tacho_needle->height()) / 2;

    tacho_numbers_offset_x = (tacho_img_width - tacho_numbers->width()) / 2;
    tacho_numbers_offset_y = (tacho_img_height - tacho_numbers->height()) / 2;

    tacho_bg_item = scene->addPixmap(*tacho_bg);
    tacho_numbers_item = scene->addPixmap(*tacho_numbers);
    tacho_needle_item = scene->addPixmap(*tacho_needle);

    tacho_numbers_item->setParentItem(tacho_bg_item);
    tacho_needle_item->setParentItem(tacho_bg_item);

    tacho_bg_item->setTransformationMode(Qt::SmoothTransformation);
    tacho_numbers_item->setTransformationMode(Qt::SmoothTransformation);
    tacho_needle_item->setTransformationMode(Qt::SmoothTransformation);

    tacho_bg_item->setTransformOriginPoint(tacho_img_width / 2, tacho_img_height / 2);
    tacho_needle_item->setTransformOriginPoint(tacho_needle->width()/2,tacho_needle->height() / 2);
    tacho_numbers_item->setTransformOriginPoint(tacho_numbers->width()/2,tacho_numbers->height() / 2);

    tacho_bg_item->setPos(tacho_bg_offset_x, tacho_bg_offset_y);



    tacho_needle_item->setPos(tacho_needle_offset_x +4 , tacho_needle_offset_y);
    tacho_numbers_item->setPos(tacho_numbers_offset_x, tacho_numbers_offset_y);

    //tacho_speed_font = new QFont("Azeret Mono", 40, 100, true);
    tacho_speed_font = new QFont("DSEG7 Classic", 60, 100, false);
    tacho_gear_font = new QFont("DSEG14 Classic", 35, 100, false);
    //tacho_gear_font->

    tacho_gear_bg_item = scene->addText("~", *tacho_gear_font);
    tacho_gear_bg_item->setDefaultTextColor(QColor(0,0,0,40));
    tacho_gear_bg_item->setPos(254,146);
    tacho_gear_bg_item->setParentItem(tacho_bg_item);

    tacho_gear_item = scene->addText("R", *tacho_gear_font);
    tacho_gear_item->setDefaultTextColor(Qt::black);
    tacho_gear_item->setPos(254,146);
    tacho_gear_item->setParentItem(tacho_bg_item);

    tacho_speed_bg_item = scene->addText("888", *tacho_speed_font);
    tacho_speed_item = scene->addText("000", *tacho_speed_font);
    tacho_speed_bg_item->setParentItem(tacho_bg_item);
    tacho_speed_item->setParentItem(tacho_bg_item);
    tacho_speed_item->setPos(152, 307);
    tacho_speed_bg_item->setPos(152,307);
    tacho_speed_item->setDefaultTextColor(Qt::black);
    tacho_speed_bg_item->setDefaultTextColor(QColor(0,0,0,40));

    set_rpm(0);
//    set_speed(0.f);

}


void tachometer::set_gear(tacho_gear_t gear)
{
    if(gear != new_gear)
    {
        this->new_gear = gear;
        this->gear_timer->stop();
        this->gear_timer->start(1000);
    }

}

void tachometer::update_gear_display()
{
    this->old_gear = this->new_gear;
    this->gear_timer->stop();
    switch(this->old_gear)
    {
    case GEAR_R:
        tacho_gear_item->setPlainText("R");
        break;
    case GEAR_1:
        tacho_gear_item->setPlainText("1");
        break;
    case GEAR_2:
        tacho_gear_item->setPlainText("2");
        break;
    case GEAR_3:
        tacho_gear_item->setPlainText("3");
        break;
    case GEAR_4:
        tacho_gear_item->setPlainText("4");
        break;
    case GEAR_5:
        tacho_gear_item->setPlainText("5");
        break;
    case GEAR_6:
        tacho_gear_item->setPlainText("6");
        break;
    case GEAR_N:
    default:
        tacho_gear_item->setPlainText("N");
        break;
    }
}

QGraphicsScene* tachometer::get_scene()
{
    return this->scene;
}

void tachometer::set_speed_pos(int x, int y)
{
     tacho_speed_item->setPos(x,y);
}

void tachometer::set_rpm(int rpm)
{
    this->rpm = rpm;
    //QFuture<void> future = QtConcurrent::run(this, &tachometer::get_angle_from_rpm);
    //this->rpm_update_watcher->setFuture(future);
    this->rpm_angle = get_angle_from_rpm();
    update_display();
}

void tachometer::update_display()
{
    tacho_needle_item->setRotation(this->rpm_angle);
    tacho_speed_item->setPlainText(this->speed_text);
}

void tachometer::set_speed_text(int speed)
{
    if(speed < 10)
    {
        this->speed_text = QString::asprintf("!!%1d", (int)speed);
    }
    else if(speed < 100)
    {
        this->speed_text = QString::asprintf("!%2d", (int)speed);
    }
    else{
        this->speed_text = QString::asprintf("%3d", (int)speed);
    }
}

void tachometer::set_speed(float speed)
{
    this->speed = speed;
    //QFuture<void> future = QtConcurrent::run(this, &tachometer::set_speed_text, this->speed);
    //this->speed_update_watcher->setFuture(future);
    set_speed_text((int)lround(speed));
    update_display();
}

float tachometer::get_angle_from_rpm()
{
    float offset = 65.f;
    float angle = this->rpm * (230.f / 7000);
    float value = offset + angle;

    this->rpm_angle = value;
    return this->rpm_angle;
}

void tachometer::set_scale(float new_scale)
{
    tacho_bg_item->setScale(new_scale);

    this->tacho_scale = new_scale;
}

float tachometer::get_scale()
{
    return this->tacho_scale;
}

int tachometer::get_rpm()
{
    return this->rpm;
}

tachometer::~tachometer()
{
    scene->clear();

//    delete tacho_bg_item;
//    delete tacho_numbers_item;
//    delete tacho_needle_item;
//    delete tacho_speed_item;
//    delete tacho_gear_item;

    delete tacho_bg;
    delete tacho_needle;
    delete tacho_numbers;

    delete tacho_speed_font;
    delete tacho_gear_font;
}

