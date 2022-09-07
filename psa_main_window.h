#ifndef PSA_MAIN_WINDOW_H
#define PSA_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QIcon>
#include <QGroupBox>
//#include "tachometer.h"
#include "psa_van_receiver.h"
#include "os_helper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class psa_main_window; }
QT_END_NAMESPACE

enum psa_selected_window{
    PSA_WINDOW_NONE = 0,
    PSA_WINDOW_INFO = 1,
    PSA_WINDOW_RADIO = 2,
    PSA_WINDOW_MUSIC = 3,
    PSA_WINDOW_AAUTO = 4,
    PSA_WINDOW_BADAPPLE = 5,
};

enum psa_selected_audio_setting{
    PSA_AUDIO_NONE = -1,
    PSA_AUDIO_BASS = 0,
    PSA_AUDIO_TREBLE = 1,
    PSA_AUDIO_LOUDNESS = 2,
    PSA_AUDIO_FADER = 3,
    PSA_AUDIO_BALANCE = 4,
    PSA_AUDIO_AUTOVOL = 5,
    PSA_AUDIO_NUM_SETTINGS = 6,
};

struct audio_settings_values_struct
{
    int8_t bass;
    int8_t treble;
    int8_t fader;
    int8_t balance;
    uint8_t auto_vol;
    uint8_t loud;
};

typedef enum psa_selected_window psa_selected_window_t;

class psa_main_window : public QMainWindow
{
    Q_OBJECT

public:
    psa_main_window(QWidget *parent = nullptr);
    ~psa_main_window();
    QTimer* screen_refresh_timer;


private:
    Ui::psa_main_window *ui;
    psa_selected_window_t selected_window;
    psa_selected_window_t new_selected_window;

    QTimer *window_timer;
    QTimer *volume_box_timer;
    QTimer *backlight_adjustment_timer;
    QIcon *radio_icon;
    QIcon *music_icon;
    QIcon *aauto_icon;
    QIcon *info_icon;


    QPixmap *radio_on_pixmap;
    QPixmap *radio_off_pixmap;
    QPixmap *music_on_pixmap;
    QPixmap *music_off_pixmap;

    os_helper *pi_helper;

    psa_van_receiver *van_handle;
    QString *radio_station;
    int speed;

    QVector<QGroupBox*> audio_settings_names_vector;

    void update_right_text();
public slots:

    void update_clock_display();

    void showEvent(QShowEvent *event);

    void refresh_screens();

    void select_window_info(bool checked);

    void select_window_radio(bool checked);

    void select_window_music(bool checked);

    void select_window_aauto(bool checked);

    void select_window_badapple();

    void display_selected_window();

    void set_main_display_speed(int speed);

    void set_main_display_radio_station(QString station);

    void show_volume_screen();

    void hide_volume_screen();

    void show_audio_settings();
    void hide_audio_settings();

    void select_audio_setting(enum psa_selected_audio_setting setting_num);

    void update_audio_settings(struct audio_settings_values_struct settings);

    void parse_engine_data(psa_engine_data_t engine_data);
    void parse_radio_data(psa_radio_data_t radio_data);
    void parse_trip_data(psa_trip_data_t trip_data);
    void parse_time_data(psa_time_data_t time_data);
    void parse_headunit_data(psa_headunit_data_t headunit_data);
    void parse_door_data(psa_door_data_t door_data);
    void parse_status_data(psa_status_data_t status_data);

    void brightness_adjustment_task();
};
#endif // PSA_MAIN_WINDOW_H
