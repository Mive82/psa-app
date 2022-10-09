#ifndef TACHOMETER_H
#define TACHOMETER_H
#include <QObject>
#include "qfuturewatcher.h"
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

typedef enum tacho_range
{
    PSA_TACHO_7K_DEFAULT = 0,
    PSA_TACHO_7K_ID7,
    PSA_TACHO_7K_ID6,
    PSA_TACHO_7K_ID5,
    PSA_TACHO_7K_ID4,
    PSA_TACHO_7K_ID3,
    PSA_TACHO_8K_DEFAULT,
} tacho_range_t;

typedef enum tacho_gear{
    GEAR_R = -1,
    GEAR_N = 0,
    GEAR_1 = 1,
    GEAR_2 = 2,
    GEAR_3 = 3,
    GEAR_4 = 4,
    GEAR_5 = 5,
    GEAR_6 = 6,
} tacho_gear_t;

class tachometer : public QObject
{
    Q_OBJECT

public:
    explicit tachometer(QObject *parent = nullptr);
    explicit tachometer(tacho_range_t range, QObject *parent = nullptr);
    ~tachometer();

    QGraphicsScene *get_scene();
    void set_scale(float scale);
    void set_rpm(int rpm);
    void set_speed(float speed);
    void set_gear(tacho_gear_t gear);
    int get_rpm();
    float get_scale();

    QFutureWatcher<void> *speed_update_watcher;
    QFutureWatcher<void> *rpm_update_watcher;

    void set_speed_pos(int x,int y);

public slots:
    void update_display();
    void update_gear_display();
    void set_gear(int gear);
    void toggle_gear_up_display();

private:
    int rpm;
    float rpm_angle;
    float speed; // 328, 290
    QString speed_text;
    tacho_range_t set_range;

    int tacho_img_width;
    int tacho_img_height;

    int tacho_needle_offset_x;
    int tacho_needle_offset_y;

    int tacho_needle_rotation_center_x;
    int tacho_needle_rotation_center_y;

    int tacho_numbers_offset_x;
    int tacho_numbers_offset_y;

    float tacho_scale;

    float start_angle;
    float end_angle;

    int tacho_redline;

    QPixmap *tacho_bg;
    QPixmap *tacho_needle;
    QPixmap *tacho_numbers;
    QPixmap *gear_up_icon;

    QVector<QPixmap> tacho_speed_pixmap_font;
    QVector<QPixmap> tacho_gear_pixmap_font;

    QFont *tacho_speed_font;
    QFont *tacho_gear_font;

    QGraphicsPixmapItem *tacho_bg_item;
    QGraphicsPixmapItem *tacho_numbers_item;
    QGraphicsPixmapItem *tacho_needle_item;
    QGraphicsPixmapItem *gear_up_icon_item;
    QGraphicsPixmapItem *tacho_gear_item_pixmap;
    QGraphicsPixmapItem *tacho_speed_item_1_pixmap;
    QGraphicsPixmapItem *tacho_speed_item_10_pixmap;
    QGraphicsPixmapItem *tacho_speed_item_100_pixmap;

    QGraphicsTextItem *tacho_speed_item;
    QGraphicsTextItem *tacho_speed_bg_item;
    QGraphicsTextItem *tacho_gear_item;
    QGraphicsTextItem *tacho_gear_bg_item;

    QGraphicsScene *scene;

    float get_angle_from_rpm();
    void create_tachometer(tacho_range_t range);
    void create_tachometer_default_7k();
    void create_tachometer_id7_7k();

    void set_speed_text(int speed);
    void update_gear_up(int rpm);

    enum tacho_gear old_gear = GEAR_R;
    enum tacho_gear new_gear = GEAR_R;

    bool gear_up_visible = false;
    int gear_up_state = 0;
    bool gear_up_grace = false; // If true, disable gear_up

    QTimer *gear_timer;
    QTimer *gear_up_blink_timer;
};

#endif // TACHOMETER_H
