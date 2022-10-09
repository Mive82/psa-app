#include "tachometer.h"
//#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QTimer>
#include <QIcon>

#include <math.h>


#include <QtConcurrent/QtConcurrentRun>

static const int gear_up_first_level = 2800;
static const int gear_up_second_level = 4000;
static const int gear_up_blink_interval_ms = 350;
static const enum tacho_gear top_gear = GEAR_5;

tachometer::tachometer(QObject *parent) : QObject{parent}
{
    create_tachometer(PSA_TACHO_7K_DEFAULT);
}

tachometer::tachometer(tacho_range_t range, QObject *parent) : QObject{parent}
{
    create_tachometer(range);
}

void tachometer::create_tachometer_default_7k()
{
    this->tacho_bg = new QPixmap(":/tachometer/tacho/background.png");
    this->tacho_numbers = new QPixmap(":/tachometer/tacho/7k.png");
    this->tacho_needle = new QPixmap(":/tachometer/tacho/needle.png");
    this->gear_up_icon = new QPixmap(QIcon(":/tachometer/tacho/gear_up.svg").pixmap(QSize(45,45)));
    this->start_angle = 65.f;
    this->end_angle = 230.f;
    this->tacho_redline = 7000;
    this->set_range = PSA_TACHO_7K_DEFAULT;


    this->tacho_img_width = tacho_bg->width();
    this->tacho_img_height = tacho_bg->height();

    int scene_w = 800;
    int scene_h = 400;

    int tacho_bg_offset_x = (scene_w - tacho_img_width) / 2;
    int tacho_bg_offset_y = (scene_h - tacho_img_height) / 2;

    this->scene->setSceneRect(0,0,scene_w, scene_h);

    this->tacho_needle_offset_x = (tacho_img_width - tacho_needle->width()) / 2;
    this->tacho_needle_offset_y = (tacho_img_height - tacho_needle->height()) / 2;

    this->tacho_numbers_offset_x = (tacho_img_width - tacho_numbers->width()) / 2;
    this->tacho_numbers_offset_y = (tacho_img_height - tacho_numbers->height()) / 2;

    this->tacho_bg_item = scene->addPixmap(*tacho_bg);
    this->tacho_numbers_item = scene->addPixmap(*tacho_numbers);
    this->tacho_needle_item = scene->addPixmap(*tacho_needle);


    this->tacho_numbers_item->setParentItem(tacho_bg_item);
    this->tacho_needle_item->setParentItem(tacho_bg_item);

    this->tacho_bg_item->setTransformationMode(Qt::SmoothTransformation);
    this->tacho_numbers_item->setTransformationMode(Qt::SmoothTransformation);
    this->tacho_needle_item->setTransformationMode(Qt::SmoothTransformation);

    this->tacho_bg_item->setTransformOriginPoint(tacho_img_width / 2, tacho_img_height / 2);
    this->tacho_needle_item->setTransformOriginPoint(tacho_needle->width()/2,tacho_needle->height() / 2);
    this->tacho_numbers_item->setTransformOriginPoint(tacho_numbers->width()/2,tacho_numbers->height() / 2);

    this->tacho_bg_item->setPos(tacho_bg_offset_x, tacho_bg_offset_y);



    this->tacho_needle_item->setPos(tacho_needle_offset_x +4 , tacho_needle_offset_y);
    this->tacho_numbers_item->setPos(tacho_numbers_offset_x, tacho_numbers_offset_y);

    //tacho_speed_font = new QFont("Azeret Mono", 40, 100, true);
    this->tacho_speed_font = new QFont("DSEG7 Classic", 60, 100, false);
    this->tacho_gear_font = new QFont("DSEG14 Classic", 35, 100, false);
    //tacho_gear_font->

    this->tacho_gear_bg_item = scene->addText("~", *tacho_gear_font);
    this->tacho_gear_bg_item->setDefaultTextColor(QColor(0,0,0,40));
    this->tacho_gear_bg_item->setPos(254,146);
    this->tacho_gear_bg_item->setParentItem(tacho_bg_item);

    this->tacho_gear_item = scene->addText("R", *tacho_gear_font);
    this->tacho_gear_item->setDefaultTextColor(Qt::black);
    this->tacho_gear_item->setPos(254,146);
    this->tacho_gear_item->setParentItem(tacho_bg_item);

    this->tacho_speed_bg_item = scene->addText("888", *tacho_speed_font);
    this->tacho_speed_item = scene->addText("000", *tacho_speed_font);
    this->tacho_speed_bg_item->setParentItem(tacho_bg_item);
    this->tacho_speed_item->setParentItem(tacho_bg_item);
    this->tacho_speed_item->setPos(152, 307);
    this->tacho_speed_bg_item->setPos(152,307);
    this->tacho_speed_item->setDefaultTextColor(Qt::black);
    this->tacho_speed_bg_item->setDefaultTextColor(QColor(0,0,0,40));

    this->gear_up_icon_item = scene->addPixmap(*this->gear_up_icon);
    this->gear_up_icon_item->setPos(210,150);
    this->gear_up_icon_item->setParentItem(tacho_bg_item);
    this->gear_up_icon_item->setVisible(gear_up_visible);

    this->set_rpm(0);

    this->set_scale(0.5f);

    return;
}

void tachometer::create_tachometer_id7_7k()
{
    this->tacho_speed_pixmap_font.resize(10);

    this->tacho_gear_pixmap_font.resize(11);

    this->tacho_speed_pixmap_font[0] = QPixmap(":/tachometer/ID7/speed/speed_digits_0.png");
    this->tacho_speed_pixmap_font[1] = QPixmap(":/tachometer/ID7/speed/speed_digits_1.png");
    this->tacho_speed_pixmap_font[2] = QPixmap(":/tachometer/ID7/speed/speed_digits_2.png");
    this->tacho_speed_pixmap_font[3] = QPixmap(":/tachometer/ID7/speed/speed_digits_3.png");
    this->tacho_speed_pixmap_font[4] = QPixmap(":/tachometer/ID7/speed/speed_digits_4.png");
    this->tacho_speed_pixmap_font[5] = QPixmap(":/tachometer/ID7/speed/speed_digits_5.png");
    this->tacho_speed_pixmap_font[6] = QPixmap(":/tachometer/ID7/speed/speed_digits_6.png");
    this->tacho_speed_pixmap_font[7] = QPixmap(":/tachometer/ID7/speed/speed_digits_7.png");
    this->tacho_speed_pixmap_font[8] = QPixmap(":/tachometer/ID7/speed/speed_digits_8.png");
    this->tacho_speed_pixmap_font[9] = QPixmap(":/tachometer/ID7/speed/speed_digits_9.png");

    this->tacho_gear_pixmap_font[0] = QPixmap(":/tachometer/ID7/gears/gear_0.png");
    this->tacho_gear_pixmap_font[1] = QPixmap(":/tachometer/ID7/gears/gear_1.png");
    this->tacho_gear_pixmap_font[2] = QPixmap(":/tachometer/ID7/gears/gear_2.png");
    this->tacho_gear_pixmap_font[3] = QPixmap(":/tachometer/ID7/gears/gear_3.png");
    this->tacho_gear_pixmap_font[4] = QPixmap(":/tachometer/ID7/gears/gear_4.png");
    this->tacho_gear_pixmap_font[5] = QPixmap(":/tachometer/ID7/gears/gear_5.png");
    this->tacho_gear_pixmap_font[6] = QPixmap(":/tachometer/ID7/gears/gear_6.png");
    this->tacho_gear_pixmap_font[7] = QPixmap(":/tachometer/ID7/gears/gear_7.png");
    this->tacho_gear_pixmap_font[8] = QPixmap(":/tachometer/ID7/gears/gear_8.png");
    this->tacho_gear_pixmap_font[9] = QPixmap(":/tachometer/ID7/gears/gear_9.png");
    this->tacho_gear_pixmap_font[10] = QPixmap(":/tachometer/ID7/gears/gear_10.png");

    this->tacho_bg = new QPixmap(":/tachometer/ID7/background.png");
    this->tacho_numbers = new QPixmap(":/tachometer/ID7/labels_8k.png");
    this->gear_up_icon = new QPixmap(":/tachometer/ID7/rev_light.png");
    this->tacho_needle = new QPixmap();

    this->tacho_needle->convertFromImage(QImage(":/tachometer/ID7/needle.png").mirrored(false,true));

    this->start_angle = 0.f;
    this->end_angle = 240.f;
    this->tacho_redline = 8000;
    this->set_range = PSA_TACHO_7K_ID7;

    this->tacho_img_width = tacho_bg->width();
    this->tacho_img_height = tacho_bg->height();

    int scene_w = 800;
    int scene_h = 400;

    int tacho_bg_offset_x = (scene_w - tacho_img_width) / 2;
    int tacho_bg_offset_y = (scene_h - tacho_img_height) / 2;

    this->scene->setSceneRect(0,0,scene_w, scene_h);

    this->tacho_numbers_offset_x = 23;
    this->tacho_numbers_offset_y = 24;

    this->tacho_needle_offset_x = 152;
    this->tacho_needle_offset_y = 259;

    float tacho_needle_rot_offset_x = (float)tacho_needle->width()/2;
    float tacho_needle_rot_offset_y = -94.f;

    float tacho_speed_offset_x = 170;
    float tacho_speed_offset_y = 114;
    float tacho_speed_width = tacho_speed_pixmap_font[0].width();

    float tacho_gear_offset_x = 152;
    float tacho_gear_offset_y = 183;

    this->tacho_speed_item_1_pixmap = scene->addPixmap(this->tacho_speed_pixmap_font[0]);
    this->tacho_speed_item_10_pixmap = scene->addPixmap(this->tacho_speed_pixmap_font[0]);
    this->tacho_speed_item_100_pixmap = scene->addPixmap(this->tacho_speed_pixmap_font[0]);

    this->tacho_gear_item_pixmap = scene->addPixmap(this->tacho_gear_pixmap_font[0]);

    this->gear_up_icon_item = scene->addPixmap(*this->gear_up_icon);

    this->tacho_speed_item_10_pixmap->hide();
    this->tacho_speed_item_100_pixmap->hide();

    this->tacho_bg_item = scene->addPixmap(*tacho_bg);
    this->tacho_numbers_item = scene->addPixmap(*tacho_numbers);
    this->tacho_needle_item = scene->addPixmap(*tacho_needle);


    this->tacho_numbers_item->setParentItem(tacho_bg_item);
    this->tacho_needle_item->setParentItem(tacho_bg_item);
    this->tacho_speed_item_1_pixmap->setParentItem(tacho_bg_item);
    this->tacho_speed_item_10_pixmap->setParentItem(tacho_bg_item);
    this->tacho_speed_item_100_pixmap->setParentItem(tacho_bg_item);
    this->tacho_gear_item_pixmap->setParentItem(tacho_bg_item);
    this->gear_up_icon_item->setParentItem(tacho_bg_item);



    this->tacho_bg_item->setTransformationMode(Qt::SmoothTransformation);
    this->tacho_numbers_item->setTransformationMode(Qt::SmoothTransformation);
    this->tacho_needle_item->setTransformationMode(Qt::SmoothTransformation);
    this->gear_up_icon_item->setTransformationMode(Qt::SmoothTransformation);


    this->tacho_speed_item_1_pixmap->setTransformationMode(Qt::SmoothTransformation);
    this->tacho_speed_item_10_pixmap->setTransformationMode(Qt::SmoothTransformation);
    this->tacho_speed_item_100_pixmap->setTransformationMode(Qt::SmoothTransformation);
    this->tacho_gear_item_pixmap->setTransformationMode(Qt::SmoothTransformation);

    this->tacho_bg_item->setTransformOriginPoint(tacho_img_width / 2, tacho_img_height / 2);
//    this->tacho_needle_item->setTransformOriginPoint(tacho_needle->width()/2,tacho_needle->height() / 2);
    this->tacho_numbers_item->setTransformOriginPoint(tacho_numbers->width()/2,tacho_numbers->height() / 2);


    this->tacho_bg_item->setPos(tacho_bg_offset_x, tacho_bg_offset_y - 30);
    this->tacho_numbers_item->setPos(tacho_numbers_offset_x, tacho_numbers_offset_y);
    this->tacho_needle_item->setPos(tacho_needle_offset_x, tacho_needle_offset_y);
    this->tacho_needle_item->setTransformOriginPoint(tacho_needle_rot_offset_x, tacho_needle_rot_offset_y);

    this->tacho_speed_item_1_pixmap->setPos(tacho_speed_offset_x, tacho_speed_offset_y);
    this->tacho_speed_item_10_pixmap->setPos(tacho_speed_offset_x - tacho_speed_width, tacho_speed_offset_y);
    this->tacho_speed_item_100_pixmap->setPos(tacho_speed_offset_x - tacho_speed_width * 2, tacho_speed_offset_y);

    this->tacho_gear_item_pixmap->setPos(tacho_gear_offset_x, tacho_gear_offset_y);
    this->gear_up_icon_item->setPos(12,12);
    this->gear_up_icon_item->setVisible(gear_up_visible);

    this->set_rpm(0);
    this->set_gear(GEAR_R);
    this->set_scale(0.85f);

//    this->tacho_speed_item_100_pixmap->setPixmap(tacho_speed_pixmap_font[1]);

}

void tachometer::create_tachometer(tacho_range_t range)
{
    this->gear_timer = new QTimer(this);
    this->gear_up_blink_timer = new QTimer(this);
    connect(this->gear_timer, &QTimer::timeout, this, &tachometer::update_gear_display);
    connect(this->gear_up_blink_timer, &QTimer::timeout, this, &tachometer::toggle_gear_up_display);
    this->speed_update_watcher = new QFutureWatcher<void>(this);
    this->rpm_update_watcher = new QFutureWatcher<void>(this);

    this->gear_up_visible = false;

    QObject::connect(this->speed_update_watcher, &QFutureWatcher<void>::finished, this, &tachometer::update_display);
    QObject::connect(this->rpm_update_watcher, &QFutureWatcher<void>::finished, this, &tachometer::update_display);
    this->scene = new QGraphicsScene();

//    tacho_bg = new QPixmap(":/tachometer/tacho/background.png");


//    QIcon gear_up_qicon_temp = QIcon(":/tachometer/tacho/gear_up.svg");



    switch(range)
    {
    case PSA_TACHO_7K_DEFAULT:
    case PSA_TACHO_8K_DEFAULT:
    default:
        return create_tachometer_default_7k();
        break;
    case PSA_TACHO_7K_ID7:
        return create_tachometer_id7_7k();
        break;
    }


//    set_speed(0.f);

}

void tachometer::toggle_gear_up_display()
{
    this->gear_up_visible = !this->gear_up_visible;
    this->gear_up_icon_item->setVisible(this->gear_up_visible);
}

void tachometer::set_gear(tacho_gear_t gear)
{

    if(gear != new_gear)
    {
        this->gear_up_grace = true;
        update_gear_up(this->rpm);
        this->new_gear = gear;
        this->gear_timer->stop();
        this->gear_timer->start(1000);
    }

}

void tachometer::set_gear(int gear)
{
    set_gear((tacho_gear_t)gear);
}

void tachometer::update_gear_display()
{

    this->gear_up_grace = false;
    this->old_gear = this->new_gear;
    update_gear_up(this->rpm);
    this->gear_timer->stop();

    switch(this->set_range)
    {
        case PSA_TACHO_7K_DEFAULT:
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
        break;

        case PSA_TACHO_7K_ID7:
            this->tacho_gear_item_pixmap->setPixmap(this->tacho_gear_pixmap_font[this->old_gear+1]);
        break;
        default:
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

void tachometer::update_gear_up(int rpm)
{


    if(this->new_gear == top_gear || this->gear_up_grace || this->old_gear < GEAR_1)
    {
        if(gear_up_state != 0)
        {

            this->gear_up_icon_item->setVisible(false);
            this->gear_up_visible = false;
            this->gear_up_blink_timer->stop();
            gear_up_state = 0;
        }
    }
    else{
        if(rpm < gear_up_first_level)
        {
            if(gear_up_state != 0)
            {

                this->gear_up_icon_item->setVisible(false);
                this->gear_up_visible = false;
                this->gear_up_blink_timer->stop();
                gear_up_state = 0;
            }
        }
        else if(rpm < gear_up_second_level)
        {
            if(gear_up_state != 1)
            {
                this->gear_up_icon_item->setVisible(true);
                this->gear_up_visible = true;
                this->gear_up_blink_timer->stop();
                gear_up_state = 1;
            }
        }
        else{
            if(gear_up_state != 2)
            {
                this->toggle_gear_up_display();
                this->gear_up_blink_timer->start(gear_up_blink_interval_ms);
                gear_up_state = 2;
            }
        }
    }
}

void tachometer::set_rpm(int rpm)
{
    this->rpm = rpm;
    this->update_gear_up(this->rpm);
    //QFuture<void> future = QtConcurrent::run(this, &tachometer::get_angle_from_rpm);
    //this->rpm_update_watcher->setFuture(future);
    this->rpm_angle = get_angle_from_rpm();
    update_display();
}

void tachometer::update_display()
{
    if(tacho_needle_item == NULL)
    {
        return;
    }
    tacho_needle_item->setRotation(this->rpm_angle);
    int temp_speed = 0;
    switch(this->set_range)
    {
        case PSA_TACHO_7K_DEFAULT:
            tacho_speed_item->setPlainText(this->speed_text);
        break;
        case PSA_TACHO_7K_ID7:
            temp_speed = (int)lround(this->speed);
            if(temp_speed < 10)
            {
                this->tacho_speed_item_10_pixmap->hide();
                this->tacho_speed_item_100_pixmap->hide();
                this->tacho_speed_item_1_pixmap->setPixmap(this->tacho_speed_pixmap_font[temp_speed%10]);
            }
            else if (temp_speed < 100)
            {
                this->tacho_speed_item_10_pixmap->show();
                this->tacho_speed_item_100_pixmap->hide();
                this->tacho_speed_item_1_pixmap->setPixmap(this->tacho_speed_pixmap_font[temp_speed%10]);
                temp_speed /= 10;
                this->tacho_speed_item_10_pixmap->setPixmap(this->tacho_speed_pixmap_font[temp_speed%10]);
            }
            else{
                this->tacho_speed_item_10_pixmap->show();
                this->tacho_speed_item_100_pixmap->show();
                this->tacho_speed_item_1_pixmap->setPixmap(this->tacho_speed_pixmap_font[temp_speed%10]);
                temp_speed /= 10;
                this->tacho_speed_item_10_pixmap->setPixmap(this->tacho_speed_pixmap_font[temp_speed%10]);
                temp_speed /= 10;
                this->tacho_speed_item_100_pixmap->setPixmap(this->tacho_speed_pixmap_font[temp_speed%10]);
            }

        break;
        default:

        break;
    }
}

void tachometer::set_speed_text(int speed)
{
    if(this->set_range != PSA_TACHO_7K_DEFAULT)
    {
        return;
    }

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
    float offset = start_angle;
    float angle = this->rpm * (end_angle / tacho_redline);
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

