#ifndef PSA_RADIO_WINDOW_H
#define PSA_RADIO_WINDOW_H

#include <QWidget>
#include <QLabel>
#include "psa_van_receiver.h"
namespace Ui {
class psa_radio_window;
}

class psa_radio_window : public QWidget
{
    Q_OBJECT

public:
    explicit psa_radio_window(QWidget *parent = nullptr);
    ~psa_radio_window();

public slots:
    void receive_radio_data(psa_radio_data_t radio_data);
    void receive_time_data(psa_time_data_t time_data);
    void receive_headunit_data(psa_headunit_data_t headunit_data);
    void receive_cd_player_data(psa_cd_player_data_t cd_data);
    void receive_presets_data(psa_preset_data_t preset_data);
    void change_clock_state();

private:
    Ui::psa_radio_window *ui;
    psa_van_receiver *van_handle = NULL;

    QTimer* clock_timer = NULL;

    void change_station(QString name);
    void change_freq(float freq);
    void change_preset(int preset);
    void change_band(enum psa_radio_band band);
    void change_radio_state(bool state);
    void change_source(enum psa_radio_source new_source);


    QString station_name;
    QVector<QString> preset_list;

    QVector<QLabel*>preset_labels;
    int last_preset = 0;
};

#endif // PSA_RADIO_WINDOW_H
