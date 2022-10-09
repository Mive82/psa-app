#include "psa_main_window.h"
#include "ui_psa_main_window.h"

#include <QGraphicsPixmapItem>
#include <QTime>
#include <QScreen>


static int clock_state = 0;
static int clock_hours = 0, clock_minutes = 0;
static int clock_total_seconds = 0;

static const int window_time_ms = 4000;

static int volume_set = 0; // Indicates if volume was ever read
static int last_volume = 0;
static bool radio_power_state = false;
static enum psa_radio_source radio_source = PSA_RADIO_NONE;
static enum psa_cd_changer_commands last_cdc_comm = PSA_CD_CHANGER_COMM_NONE;

static enum psa_car_state last_state = PSA_STATE_UNKNOWN;

static enum psa_selected_audio_setting old_audio_setting = PSA_AUDIO_NONE;
//static int audio_menu_open = 0;

static int last_brightness = 110;

const static int low_brightness = 55;
const static int mid_brightness = 120;
const static int high_brightness = 180;

const static int transition_start_evening = 17*3600;
const static int transition_end_evening = 19.5 * 3600;

const static int transition_start_morning = 5 * 3600;
const static int transition_end_morning = 6.5 * 3600;

static int display_status = 1;

static const QString stylesheet_selected = "QGroupBox{\n/*	background: none;\n	border: 3px solid white;\n	border-radius: 15px;*/\n	border:none;\n	background-color: #556fde;\n	border-radius: 15px;\n}\n\nQLabel{\n	\n	color: white;\n}";
static const QString stylesheet_unselected = "QGroupBox{\n/*	background: none;\n	border: 3px solid white;\n	border-radius: 15px;*/\n	border:none;\n\n	background: rgb(57,57,57);\n	border-radius: 15px;\n}\n\nQLabel{\n	\n	color: white;\n}";



psa_main_window::psa_main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::psa_main_window)
{
    ui->setupUi(this);

    //QCoreApplication::processEvents();

    ui->speed_slider->hide();
    ui->verticalSlider->hide();
    ui->mute_label->hide();
    ui->door_alerts_box->hide();
    ui->radio_volume_box->hide();
    ui->audio_settings_box->hide();
    ui->badapple_button->hide();

    this->audio_settings_names_vector.resize(PSA_AUDIO_NUM_SETTINGS);
    this->audio_settings_names_vector[PSA_AUDIO_BASS] = this->ui->bass_box;
    this->audio_settings_names_vector[PSA_AUDIO_TREBLE] = this->ui->treble_box;
    this->audio_settings_names_vector[PSA_AUDIO_LOUDNESS] = this->ui->loud_box;
    this->audio_settings_names_vector[PSA_AUDIO_FADER] = this->ui->fader_box;
    this->audio_settings_names_vector[PSA_AUDIO_BALANCE] = this->ui->balance_box;
    this->audio_settings_names_vector[PSA_AUDIO_AUTOVOL] = this->ui->auto_vol_box;

    this->select_audio_setting(PSA_AUDIO_NONE);

    for(int i = 0; i< PSA_AUDIO_NUM_SETTINGS; ++i)
    {
        this->audio_settings_names_vector[i]->setStyleSheet(stylesheet_unselected);
    }

    this->pi_helper = new os_helper(this);

//    this->pi_helper->set_lcd_brightness(last_brightness);

    this->volume_box_timer = new QTimer(this);


//    this->brightness_adjustment_task();

//    this->volume_box_timer->start(5000);
    this->backlight_adjustment_timer = new QTimer(this);
    connect(this->backlight_adjustment_timer, &QTimer::timeout, this, &psa_main_window::brightness_adjustment_task);
    this->backlight_adjustment_timer->start(5000);

    this->speed = 0;
    this->radio_station = new QString("");

    this->selected_window = PSA_WINDOW_NONE;
    this->new_selected_window = PSA_WINDOW_RADIO;

    this->screen_refresh_timer = new QTimer(this);
    ui->badapple_widget->setVisible(false);


// #define PSA_SIMULATE
#ifndef PSA_UI_SIMULATE

    this->van_handle = new psa_van_receiver(this);
    this->van_handle->receive_radio_packets(true, 103);
    this->van_handle->receive_engine_packets(true, 10);
    this->van_handle->receive_trip_packets(true, 600);
    this->van_handle->receive_dash_packets(true);
    this->van_handle->receive_time_packets();
    this->van_handle->receive_headunit_packets(true, 52);
    this->van_handle->receive_cd_player_packets(true, 300);
    this->van_handle->receive_door_packets(true, 100);
    this->van_handle->receive_status_packet(true, 90);
    this->van_handle->receive_preset_packet();
    QObject::connect(this->van_handle, &psa_van_receiver::radio_data_changed, this->ui->radio_widget, &psa_radio_window::receive_radio_data);
//    QObject::connect(this->van_handle, &psa_van_receiver::engine_data_changed, this->ui->rpm_widget, &tachometer_window::receive_engine_data);
    QObject::connect(this->van_handle, &psa_van_receiver::headunit_data_changed, this->ui->radio_widget, &psa_radio_window::receive_headunit_data);
    QObject::connect(this->van_handle, &psa_van_receiver::time_data_changed, this->ui->radio_widget, &psa_radio_window::receive_time_data);
    QObject::connect(this->van_handle, &psa_van_receiver::cd_player_data_changed, this->ui->radio_widget, &psa_radio_window::receive_cd_player_data);
    QObject::connect(this->van_handle, &psa_van_receiver::dash_data_changed, this->ui->rpm_widget, &tachometer_window::receive_dash_data);
    QObject::connect(this->van_handle, &psa_van_receiver::preset_data_changed, this->ui->radio_widget, &psa_radio_window::receive_presets_data);
    QObject::connect(this->van_handle, &psa_van_receiver::trip_data_changed, this->ui->rpm_widget, &tachometer_window::receive_trip_data);


    QObject::connect(this->van_handle, &psa_van_receiver::engine_data_changed, this, &psa_main_window::parse_engine_data);
    QObject::connect(this->van_handle, &psa_van_receiver::radio_data_changed, this, &psa_main_window::parse_radio_data);
    QObject::connect(this->van_handle, &psa_van_receiver::trip_data_changed, this, &psa_main_window::parse_trip_data);
    QObject::connect(this->van_handle, &psa_van_receiver::time_data_changed, this, &psa_main_window::parse_time_data);
    QObject::connect(this->van_handle, &psa_van_receiver::headunit_data_changed, this, &psa_main_window::parse_headunit_data);

    QObject::connect(this->van_handle, &psa_van_receiver::door_data_changed, this, &psa_main_window::parse_door_data);
    QObject::connect(this->van_handle, &psa_van_receiver::status_data_changed, this, &psa_main_window::parse_status_data);

#endif

    QObject::connect(ui->radio_button, &QPushButton::clicked, this, &psa_main_window::select_window_radio);
    QObject::connect(ui->music_button, &QPushButton::clicked, this, &psa_main_window::select_window_music);
    QObject::connect(ui->info_button, &QPushButton::clicked, this, &psa_main_window::select_window_info);
    QObject::connect(ui->android_auto_button, &QPushButton::clicked, this, &psa_main_window::select_window_badapple);

    QObject::connect(ui->verticalSlider, &QSlider::valueChanged, ui->rpm_widget, &tachometer_window::change_tacho_angle);
    QObject::connect(ui->speed_slider, &QSlider::valueChanged, ui->rpm_widget, &tachometer_window::change_speed);
    QObject::connect(ui->speed_slider, &QSlider::valueChanged, this, &psa_main_window::set_main_display_speed);
    QObject::connect(this->volume_box_timer, &QTimer::timeout, this, &psa_main_window::hide_volume_screen);

    QObject::connect(this->ui->music_widget, &psa_music_window::media_loaded, this, &psa_main_window::handle_music_on_resume);



    this->ui->music_widget->set_van_handle(this->van_handle);
    this->ui->rpm_widget->set_van_handle(this->van_handle);
    //QObject::connect(this->screen_refresh_timer, &QTimer::timeout ,this, &psa_main_window::update_clock_display);
    //screen_refresh_timer->start(1000);
    //update_clock_display();
   // display_selected_window();

//    QFont right_font("DSEG14 Classic", 25, 100, false);
//    QFont left_font("DSEG14 Classic", 30, 100, false);
//    ui->right_text_label->setFont(right_font);
//    ui->left_text_label->setFont(left_font);

    // ui->left_text_label->setText("!5.5");
}

void psa_main_window::update_clock_display()
{
    return;
//    if(clock_state)
//    {
//        this->ui->time_label->setText(QString::asprintf("%02d:%02d", clock_hours, clock_minutes));
//        clock_state = 0;
//    }
//    else{
//        this->ui->time_label->setText(QString::asprintf("%02d %02d", clock_hours, clock_minutes));
//        clock_state = 1;
//    }
}

void psa_main_window::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    //display_selected_window();
}

void psa_main_window::refresh_screens()
{
    display_selected_window();
}

void psa_main_window::handle_music_on_resume()
{
    if(radio_source == PSA_RADIO_EXTERNAL)
    {
        this->ui->music_widget->resume_player();
    }
}

void psa_main_window::display_selected_window()
{

    this->selected_window = this->new_selected_window;

    switch(this->selected_window)
    {
        case PSA_WINDOW_RADIO:
            ui->radio_button->setChecked(true);
            ui->music_button->setChecked(false);
            ui->info_button->setChecked(false);
            ui->rpm_widget->setVisible(false);
            //ui->badapple_button->setVisible(true);
            ui->music_widget->setVisible(false);
            ui->radio_widget->setVisible(true);
            ui->badapple_widget->hide();
            ui->badapple_widget->stop_playback();
            this->van_handle->receive_engine_packets(true, 50);
            this->van_handle->receive_radio_packets(true, 103);
            if(radio_power_state)
            {
                this->ui->time_label->show();
            }
            else{
                this->ui->time_label->hide();
            }
//            ui->rpm_widget->hide();
//            ui->music_widget->hide();
//            ui->radio_widget->show();
        break;
        case PSA_WINDOW_MUSIC:
            ui->time_label->show();
            ui->radio_button->setChecked(false);
            ui->music_button->setChecked(true);
            ui->info_button->setChecked(false);
            //ui->badapple_button->setVisible(true);
            ui->rpm_widget->setVisible(false);
            ui->music_widget->setVisible(true);
            ui->radio_widget->setVisible(false);
            ui->badapple_widget->hide();
            ui->badapple_widget->stop_playback();
            this->van_handle->receive_engine_packets(true, 50);
            this->van_handle->receive_radio_packets(false);
//            ui->rpm_widget->hide();
//            ui->radio_widget->hide();
//            ui->music_widget->show();
        break;
        case PSA_WINDOW_INFO:
            ui->time_label->show();
            ui->radio_button->setChecked(false);
            ui->music_button->setChecked(false);
            ui->info_button->setChecked(true);
            //ui->badapple_button->setVisible(true);
            ui->rpm_widget->setVisible(true);
            ui->music_widget->setVisible(false);
            ui->radio_widget->setVisible(false);
            ui->badapple_widget->hide();
            ui->badapple_widget->stop_playback();
            this->van_handle->receive_engine_packets(true, 13);
            this->van_handle->receive_radio_packets(true, 103);
//            ui->radio_widget->hide();
//            ui->music_widget->hide();
//            ui->rpm_widget->show();
        break;
        case PSA_WINDOW_BADAPPLE:
            ui->time_label->show();
            ui->radio_button->setChecked(false);
            ui->music_button->setChecked(false);
            ui->info_button->setChecked(false);
            ui->badapple_button->setVisible(false);
            ui->rpm_widget->setVisible(false);
            ui->music_widget->setVisible(false);
            ui->radio_widget->setVisible(false);
            ui->badapple_widget->show();
            ui->badapple_widget->start_playback();
            this->van_handle->receive_engine_packets(true, 50);
            this->van_handle->receive_radio_packets(false);
            break;
        default:
        break;
    }

    update_right_text();
}

void psa_main_window::update_right_text()
{
    switch(this->selected_window)
    {
        case PSA_WINDOW_RADIO:
        case PSA_WINDOW_MUSIC:
            if(this->speed < 10)
            {
                ui->right_text_label->setText(QString::asprintf("!!%1d km/h", this->speed));
            }
            else if (this->speed < 100)
            {
                ui->right_text_label->setText(QString::asprintf("!%2d km/h", this->speed));
            }
            else{
                ui->right_text_label->setText(QString::asprintf("%3d km/h", this->speed));
            }
        break;
        case PSA_WINDOW_INFO:
            if(radio_power_state == false)
            {
                ui->right_text_label->setText("OFF");
            }
            else{
                switch(radio_source)
                {
                case PSA_RADIO_NONE:
                default:
                    ui->right_text_label->setText("OFF");
                    break;
                case PSA_RADIO_TUNER:
                    ui->right_text_label->setText(*this->radio_station);
                    break;
                case PSA_RADIO_INTERNAL:
                    ui->right_text_label->setText("CD");
                    break;
                case PSA_RADIO_EXTERNAL:
                    ui->right_text_label->setText("USB");
                    break;
                case PSA_RADIO_NAVIGATION:
                    ui->right_text_label->setText("NAVI");
                }
            }

        break;
        default:
            ui->right_text_label->setText("");
        break;
    }
}

void psa_main_window::set_main_display_speed(int speed)
{
    this->speed = speed;
    update_right_text();
}

void psa_main_window::set_main_display_radio_station(QString radio_station)
{
    *this->radio_station = radio_station;
    update_right_text();
}

void psa_main_window::parse_engine_data(psa_engine_data_t engine_data)
{
    if(engine_data.speed != 0xffff){
        this->speed = engine_data.speed / 100;
        update_right_text();
    }
}

void psa_main_window::parse_trip_data(psa_trip_data_t trip_data)
{
    int fuel_raw = trip_data.current_fuel_consumption;

    if(fuel_raw == 0xffff)
    {
        ui->left_text_label->setText("!!.!");
        return;
    }

    float fuel = (float)trip_data.current_fuel_consumption / 10;

    if(fuel < 10)
    {
        ui->left_text_label->setText(QString::asprintf("!%.1f", fuel));
    }
    else{
        ui->left_text_label->setText(QString::asprintf("%.1f", fuel));
    }
}

void psa_main_window::brightness_adjustment_task()
{
    if(display_status == 0)
    {
//        this->pi_helper->set_lcd_brightness(0);
        return;
    }
    if(last_state != PSA_STATE_ENGINE_ON && last_brightness > mid_brightness)
    {
        this->pi_helper->set_lcd_brightness(mid_brightness);
        return;
    }
    if(clock_state == 0)
    {
        if(mid_brightness != last_brightness)
        {
            this->pi_helper->set_lcd_brightness(mid_brightness);
            last_brightness = mid_brightness;
        }
    }
    else{
        if(clock_total_seconds < transition_start_morning)
        {
            if(low_brightness != last_brightness)
            {
                this->pi_helper->set_lcd_brightness(low_brightness);
                last_brightness = low_brightness;
            }
        }
        else if (clock_total_seconds <= transition_end_morning){
            const float brightness_delta = high_brightness - low_brightness;
            const float morning_transition_delta = transition_end_morning - transition_start_morning;
            const int current_delta = clock_total_seconds - transition_start_morning;
            const int new_brightness = ((brightness_delta / morning_transition_delta) * current_delta) + low_brightness;
            if(new_brightness != last_brightness)
            {
                this->pi_helper->set_lcd_brightness(new_brightness);
                last_brightness = new_brightness;
            }
        }
        else if (clock_total_seconds < transition_start_evening)
        {
            if(high_brightness != last_brightness)
            {
                this->pi_helper->set_lcd_brightness(high_brightness);
                last_brightness = high_brightness;
            }
        }
        else if (clock_total_seconds <= transition_end_evening)
        {
            const float brightness_delta = high_brightness - low_brightness;
            const float evening_transition_delta = transition_end_evening - transition_start_evening;
            const int current_delta = clock_total_seconds - transition_start_evening;
            const int new_brightness = -((brightness_delta / evening_transition_delta) * current_delta) + high_brightness;
            if(new_brightness != last_brightness)
            {
                this->pi_helper->set_lcd_brightness(new_brightness);
                last_brightness = new_brightness;
            }
        }
        else{
            this->pi_helper->set_lcd_brightness(low_brightness);
            last_brightness = low_brightness;
        }
    }

    //fprintf(stdout, "Time: %d Brightness: %d\n",clock_total_seconds , last_brightness);
    //fflush(stdout);
}

void psa_main_window::parse_radio_data(psa_radio_data_t radio_data)
{


    *this->radio_station = QString::asprintf("%s", radio_data.station);
    update_right_text();
}

void psa_main_window::parse_headunit_data(psa_headunit_data_t headunit_data)
{
    int new_volume = (int)headunit_data.volume;
    bool new_radio_power_state = headunit_data.unit_powered_on;
    enum psa_radio_source new_radio_source = (enum psa_radio_source)headunit_data.source;

    if(new_radio_power_state != radio_power_state)
    {
        if(new_radio_power_state == false)
        {
            this->ui->music_widget->pause_player();
        }
        else{
            if(new_radio_source == PSA_RADIO_EXTERNAL || radio_source == PSA_RADIO_EXTERNAL)
            {
                this->ui->music_widget->resume_player();
            }
        }
    }

    if(new_radio_source != radio_source)
    {
        switch(new_radio_source)
        {
        case PSA_RADIO_NONE:
        case PSA_RADIO_TUNER:
        case PSA_RADIO_INTERNAL:
            this->ui->music_widget->pause_player();
            if(this->selected_window == PSA_WINDOW_MUSIC || this->selected_window == PSA_WINDOW_BADAPPLE)
            {
                this->new_selected_window = PSA_WINDOW_RADIO;
            }
            break;
        case PSA_RADIO_EXTERNAL:
            this->ui->music_widget->resume_player();
            if(this->selected_window == PSA_WINDOW_RADIO || this->selected_window == PSA_WINDOW_BADAPPLE)
            {
                this->new_selected_window = PSA_WINDOW_MUSIC;
            }
        case PSA_RADIO_NAVIGATION:
        default:
            break;
        }

        display_selected_window();

    }

    if(this->selected_window == PSA_WINDOW_RADIO)
    {
        if(new_radio_power_state)
        {
            this->ui->time_label->show();
        }
        else{
            this->ui->time_label->hide();
        }
    }

    if(volume_set)
    {
        if(new_volume != last_volume)
        {
            last_volume = new_volume;
            this->show_volume_screen();
        }
    }
    else{
        last_volume = new_volume;
        volume_set = 1;
    }

    if(headunit_data.muted)
    {
        this->ui->mute_label->show();
    }
    else{
        this->ui->mute_label->hide();
    }

    this->update_audio_settings({
                                 .bass = headunit_data.bass,
                                 .treble = headunit_data.treble,
                                 .fader = headunit_data.fader,
                                 .balance = headunit_data.balance,
                                 .auto_vol = headunit_data.auto_volume,
                                 .loud = headunit_data.loudness_on
    });

    if(headunit_data.settings_menu_open)
    {
        this->show_audio_settings();
        if(headunit_data.settings_changing.auto_volume_changing)
        {
            this->select_audio_setting(PSA_AUDIO_AUTOVOL);
        }
        else if(headunit_data.settings_changing.balance_changing)
        {
            this->select_audio_setting(PSA_AUDIO_BALANCE);
        }
        else if (headunit_data.settings_changing.bass_changing)
        {
            this->select_audio_setting(PSA_AUDIO_BASS);
        }
        else if (headunit_data.settings_changing.fader_changing)
        {
            this->select_audio_setting(PSA_AUDIO_FADER);
        }
        else if(headunit_data.settings_changing.loudness_changing)
        {
            this->select_audio_setting(PSA_AUDIO_LOUDNESS);
        }
        else if(headunit_data.settings_changing.treble_changing)
        {
            this->select_audio_setting(PSA_AUDIO_TREBLE);
        }
        else{
            this->select_audio_setting(PSA_AUDIO_NONE);
        }
    }
    else{
        this->hide_audio_settings();
    }

    radio_power_state = new_radio_power_state;
    radio_source = new_radio_source;
}

void psa_main_window::parse_time_data(psa_time_data_t time_data)
{
    if(time_data.valid_time==0)
    {
        this->ui->time_label->setText("");
        clock_state = 0;
    }
    else{
        this->ui->time_label->setText(QString::asprintf("%02d:%02d", time_data.hour, time_data.minutes));
        clock_hours = time_data.hour;
        clock_minutes = time_data.minutes;
        clock_total_seconds = (clock_hours * 3600) + (clock_minutes * 60) + time_data.seconds;

        if(clock_state == 0)
        {
            clock_state = 1;
            this->brightness_adjustment_task();
        }
        clock_state = 1;

    }
}

void psa_main_window::parse_door_data(psa_door_data_t door_data)
{
    if(door_data.door_open)
    {
        this->ui->door_alerts_box->setVisible(true);

        this->ui->doors_base->setVisible(true);
        this->ui->doors_front_left->setVisible((bool)door_data.front_left);
        this->ui->doors_front_right->setVisible((bool)door_data.front_right);
        this->ui->doors_rear_left->setVisible((bool)door_data.rear_left);
        this->ui->doors_rear_right->setVisible((bool)door_data.rear_right);
        this->ui->doors_boot->setVisible((bool)door_data.boot_lid);
        this->ui->doors_hood->setVisible((bool)door_data.hood);
    }
    else{
        this->ui->door_alerts_box->setVisible(false);
    }
}

void psa_main_window::parse_status_data(psa_status_data_t status_data)
{
    if(status_data.car_state != last_state)
    {

        switch(status_data.car_state)
        {
        case PSA_STATE_CAR_OFF:
        case PSA_STATE_CAR_LOCKED:
        case PSA_STATE_CAR_SLEEP:
        case PSA_STATE_ECONOMY_MODE:
        case PSA_STATE_UNKNOWN:
        default:
            this->ui->music_widget->stop_player();
            this->pi_helper->set_lcd_brightness(0);
            display_status = 0;
            break;
        case PSA_STATE_CRANKING:
            this->pi_helper->set_lcd_brightness(0);
            display_status = 0;
            break;
        case PSA_STATE_ACCESSORY:
        case PSA_STATE_IGNITION:
        case PSA_STATE_RADIO_ON:
            if(last_state <= PSA_STATE_CAR_OFF)
            {
                this->ui->music_widget->refresh_root_music_directory();
            }
            else{
                handle_music_on_resume();
            }
            if(last_brightness > mid_brightness)
            this->pi_helper->set_lcd_brightness(mid_brightness);
            display_status = 1;
            break;
        case PSA_STATE_ENGINE_ON:

            if(last_state <= PSA_STATE_CAR_OFF)
            {
                this->ui->music_widget->refresh_root_music_directory();
            }
            else{
                handle_music_on_resume();
            }
            this->brightness_adjustment_task();
            display_status = 1;
            break;
        }

        last_state = (enum psa_car_state)status_data.car_state;
    }

    enum psa_cd_changer_commands new_cdc_comm = (enum psa_cd_changer_commands)status_data.cd_changer_command;

    if(new_cdc_comm != last_cdc_comm)
    {
        switch(new_cdc_comm)
        {
        default:
        case PSA_CD_CHANGER_COMM_NONE:
            break;
        case PSA_CD_CHANGER_COMM_NEXT_TRACK:
            this->ui->music_widget->next_track();
            break;
        case PSA_CD_CHANGER_COMM_PREV_TRACK:
            this->ui->music_widget->prev_track();
            break;
        case PSA_CD_CHANGER_COMM_PAUSE:
            //this->ui->music_widget->pause_player();
            break;
        case PSA_CD_CHANGER_COMM_PLAY:
            this->ui->music_widget->resume_player();
            break;
        }

        last_cdc_comm = new_cdc_comm;
    }
}

void psa_main_window::show_volume_screen()
{
    this->ui->radio_volume_slider->setValue(last_volume);
    this->ui->radio_volume_label->setText(QString::asprintf("%d", last_volume));
    this->ui->radio_volume_box->show();
    this->volume_box_timer->stop();
    this->volume_box_timer->start(window_time_ms);
    return;
}

void psa_main_window::hide_volume_screen()
{
    this->volume_box_timer->stop();
    this->ui->radio_volume_box->hide();
    return;
}

void psa_main_window::select_audio_setting(enum psa_selected_audio_setting setting_num)
{

    if (setting_num != old_audio_setting)
    {
        if(setting_num == PSA_AUDIO_NONE)
        {
            for(int i = 0; i< PSA_AUDIO_NUM_SETTINGS; ++i)
            {
                this->audio_settings_names_vector[i]->setStyleSheet(stylesheet_unselected);
            }
        }
        else {
            this->audio_settings_names_vector[setting_num]->setStyleSheet(stylesheet_selected);
            if(old_audio_setting != PSA_AUDIO_NONE)
            {
                this->audio_settings_names_vector[old_audio_setting]->setStyleSheet(stylesheet_unselected);
            }
        }

        old_audio_setting = setting_num;
    }

}

void psa_main_window::show_audio_settings()
{
    this->ui->audio_settings_box->show();
}

void psa_main_window::hide_audio_settings()
{
    this->select_audio_setting(PSA_AUDIO_NONE);
    this->ui->audio_settings_box->hide();
}

void psa_main_window::update_audio_settings(struct audio_settings_values_struct settings)
{
    if(settings.auto_vol)
    {
        this->ui->auto_vol_value->setText("ON");
    }
    else{
        this->ui->auto_vol_value->setText("OFF");
    }

    if(settings.loud)
    {
        this->ui->loud_value->setText("ON");
    }
    else{
        this->ui->loud_value->setText("OFF");
    }

    if(settings.balance > 0)
    {
        this->ui->balance_value->setText(QString::asprintf("R %d", settings.balance));
    }
    else if (settings.balance < 0)
    {
        this->ui->balance_value->setText(QString::asprintf("L %d", -settings.balance));
    }
    else{
        this->ui->balance_value->setText("0");
    }

    if(settings.fader > 0)
    {
        this->ui->fader_value->setText(QString::asprintf("F %d", settings.fader));
    }
    else if (settings.fader < 0)
    {
        this->ui->fader_value->setText(QString::asprintf("B %d", -settings.fader));
    }
    else{
        this->ui->fader_value->setText("0");
    }


    this->ui->bass_value->setText(QString::asprintf("%d", settings.bass));
    this->ui->treble_value->setText(QString::asprintf("%d", settings.treble));

}

void psa_main_window::select_window_aauto(bool checked)
{
    Q_UNUSED(checked);
    return;
}

void psa_main_window::select_window_info(bool checked)
{

    Q_UNUSED(checked);
    this->new_selected_window = PSA_WINDOW_INFO;
    display_selected_window();

    return;
}

void psa_main_window::select_window_music(bool checked)
{
    Q_UNUSED(checked);
    this->new_selected_window = PSA_WINDOW_MUSIC;
    display_selected_window();
    return;
}

void psa_main_window::select_window_radio(bool checked)
{
    Q_UNUSED(checked);
    this->new_selected_window = PSA_WINDOW_RADIO;
    display_selected_window();
    return;
}

void psa_main_window::select_window_badapple()
{
//    QPixmap screen_pixmap = screen()->grabWindow(0);

//    screen_pixmap.save("screen.png");

    this->new_selected_window = PSA_WINDOW_BADAPPLE;
    this->ui->music_widget->pause_player();
    display_selected_window();
    return;
}

psa_main_window::~psa_main_window()
{
    delete ui;
    //delete van_handle;
}

