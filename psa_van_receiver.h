#ifndef PSA_VAN_RECEIVER_H
#define PSA_VAN_RECEIVER_H

#include <QObject>
#include <QTimer>
#include "libpsa/header/psa/psa_packet_defs.h"
#include "qfuturewatcher.h"


class psa_van_receiver : public QObject
{
    Q_OBJECT
public:
    explicit psa_van_receiver(QObject *parent = nullptr);
    ~psa_van_receiver();

    void receive_engine_packets(bool receive = true, int msec = 20);
    void receive_radio_packets(bool receive = true, int msec = 400);
    void receive_door_packets(bool receive = true, int msec = 150);
    void receive_dash_packets(bool receive = true, int msec = 500);
    void receive_trip_packets(bool receive = true, int msec = 900);
    void receive_time_packets(bool receive = true, int msed = 200);
    void receive_headunit_packets(bool receive = true, int msec = 450);
    void receive_cd_player_packets(bool receive = true, int msec = 400);
    void receive_vin_packets(bool receive = true, int msec = 1500);
    void receive_status_packet(bool receive = true, int msec = 300);
    void receive_preset_packet(bool receive = true, int msec = 500);

signals:
    void engine_data_changed(psa_engine_data_t engine_data);
    void radio_data_changed(psa_radio_data_t radio_data);
    void door_data_changed(psa_door_data_t door_data);
    void dash_data_changed(psa_dash_data_t dash_data);
    void trip_data_changed(psa_trip_data_t trip_data);
    void vin_changed(psa_vin_data_t vin_data);
    void status_data_changed(psa_status_data_t status_data);

    void preset_data_changed(psa_preset_data_t preset_data);

    void cd_player_data_changed(psa_cd_player_data_t cd_data);
    void time_data_changed(psa_time_data_t time_data);
    void headunit_data_changed(psa_headunit_data_t headunit_data);

private:
    QMutex mutex;
    struct psa_engine_data* engine_data = NULL;
    struct psa_radio_data* radio_data = NULL;
    struct psa_door_data* door_data = NULL;
    struct psa_dash_data* dash_data = NULL;
    struct psa_trip_data* trip_data = NULL;
    struct psa_vin_data* vin_data = NULL;
    struct psa_time_data* time_data = NULL;
    struct psa_headunit_data* headunit_data = NULL;
    struct psa_cd_player_data* cd_data = NULL;
    struct psa_status_data* status_data = NULL;
    struct psa_preset_data* preset_data = NULL;

    struct psa_engine_data* new_engine_data = NULL;
    struct psa_radio_data* new_radio_data = NULL;
    struct psa_door_data* new_door_data = NULL;
    struct psa_dash_data* new_dash_data = NULL;
    struct psa_trip_data* new_trip_data = NULL;
    struct psa_vin_data* new_vin_data = NULL;
    struct psa_time_data* new_time_data = NULL;
    struct psa_headunit_data* new_headunit_data = NULL;
    struct psa_cd_player_data* new_cd_data = NULL;
    struct psa_status_data* new_status_data = NULL;
    struct psa_preset_data* new_preset_data = NULL;


    QString const port = "/dev/ttyAMA0";

    QTimer *engine_timer = NULL;
    QTimer *radio_timer = NULL;
    QTimer *door_timer = NULL;
    QTimer *dash_timer = NULL;
    QTimer *trip_timer = NULL;
    QTimer *vin_timer = NULL;
    QTimer *time_timer = NULL;
    QTimer *headunit_timer = NULL;
    QTimer *cd_timer = NULL;
    QTimer *status_timer = NULL;
    QTimer *preset_timer = NULL;

    QFutureWatcher<void> *engine_data_future_watcher = NULL;
    QFutureWatcher<void> *radio_data_future_watcher = NULL;
    QFutureWatcher<void> *door_data_future_watcher = NULL;
    QFutureWatcher<void> *dash_data_future_watcher = NULL;
    QFutureWatcher<void> *trip_data_future_watcher = NULL;
    QFutureWatcher<void> *time_data_future_watcher = NULL;
    QFutureWatcher<void> *headunit_data_future_watcher = NULL;
    QFutureWatcher<void> *cd_data_future_watcher = NULL;
    QFutureWatcher<void> *vin_data_future_watcher = NULL;
    QFutureWatcher<void> *status_data_future_watcher = NULL;
    QFutureWatcher<void> *preset_data_future_watcher = NULL;

    void queue_engine_data_async();
    void queue_radio_data_async();
    void queue_trip_data_async();
    void queue_time_data_async();
    void queue_headunit_data_async();
    void queue_cd_data_async();
    void queue_door_data_async();
    void queue_dash_data_async();
    void queue_vin_data_async();
    void queue_status_data_async();
    void queue_preset_data_async();

    void queue_cdc_packet_async(psa_cd_changer_data_t cdc_data);
    void queue_trip_reset_async(psa_trip_reset_data_t trip_data);


    bool receiving_engine_data = false;
    bool receiving_radio_data = false;
    bool receiving_trip_data = false;
    bool receiving_door_data = false;
    bool receiving_dash_data = false;
    bool receiving_vin_data = false;
    bool receiving_time_data = false;
    bool receiving_headunit_data = false;
    bool receiving_cd_data = false;
    bool receiving_status_data = false;
    bool receiving_preset_data = false;

public slots:
    void get_engine_data();
    void get_radio_data();
    void get_trip_data();
    void get_time_data();
    void get_headunit_data();
    void get_cd_data();
    void get_door_data();
    void get_dash_data();
    void get_vin_data();
    void get_status_data();
    void get_preset_data();


    void engine_data_received();
    void radio_data_received();
    void trip_data_received();
    void time_data_received();
    void headunit_data_received();
    void cd_data_received();
    void door_data_received();
    void dash_data_received();
    void vin_data_received();
    void status_data_received();
    void preset_data_received();

    void send_cdc_data(psa_cd_changer_data_t cdc_data);
    void send_trip_reset(psa_trip_reset_data_t trip_reset);

};

#endif // PSA_VAN_RECEIVER_H
