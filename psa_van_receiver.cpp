#include "psa_van_receiver.h"

#include "libpsa/header/psa/psa_receive.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

psa_van_receiver::psa_van_receiver(QObject *parent)
    : QObject{parent}
{
    this->engine_data = NULL;
    this->new_engine_data = NULL;

    this->trip_data = NULL;
    this->new_trip_data = NULL;

    this->radio_data = NULL;
    this->new_radio_data = NULL;

    this->time_data = NULL;
    this->new_time_data = NULL;

    this->headunit_data = NULL;
    this->new_headunit_data = NULL;

    this->cd_data = NULL;
    this->new_cd_data = NULL;


    psa_init(this->port.toStdString().c_str());
    this->engine_timer = new QTimer(this);
    this->radio_timer = new QTimer(this);
    this->trip_timer = new QTimer(this);
    this->time_timer = new QTimer(this);
    this->headunit_timer = new QTimer(this);
    this->cd_timer = new QTimer(this);
    this->door_timer = new QTimer(this);
    this->dash_timer = new QTimer(this);
    this->vin_timer = new QTimer(this);
    this->status_timer = new QTimer(this);
    this->preset_timer = new QTimer(this);


    this->engine_data_future_watcher = new QFutureWatcher<void>(this);
    this->radio_data_future_watcher = new QFutureWatcher<void>(this);
    this->trip_data_future_watcher = new QFutureWatcher<void>(this);
    this->time_data_future_watcher = new QFutureWatcher<void>(this);
    this->headunit_data_future_watcher = new QFutureWatcher<void>(this);
    this->cd_data_future_watcher = new QFutureWatcher<void>(this);
    this->door_data_future_watcher = new QFutureWatcher<void>(this);
    this->dash_data_future_watcher = new QFutureWatcher<void>(this);
    this->vin_data_future_watcher = new QFutureWatcher<void>(this);
    this->status_data_future_watcher = new QFutureWatcher<void>(this);
    this->preset_data_future_watcher = new QFutureWatcher<void>(this);


    connect(this->engine_timer, &QTimer::timeout, this, &psa_van_receiver::get_engine_data);
    connect(this->radio_timer, &QTimer::timeout, this, &psa_van_receiver::get_radio_data);
    connect(this->trip_timer, &QTimer::timeout, this, &psa_van_receiver::get_trip_data);
    connect(this->time_timer, &QTimer::timeout, this, &psa_van_receiver::get_time_data);
    connect(this->headunit_timer, &QTimer::timeout, this, &psa_van_receiver::get_headunit_data);
    connect(this->cd_timer, &QTimer::timeout, this, &psa_van_receiver::get_cd_data);
    connect(this->door_timer, &QTimer::timeout, this, &psa_van_receiver::get_door_data);
    connect(this->dash_timer, &QTimer::timeout, this, &psa_van_receiver::get_dash_data);
    connect(this->vin_timer, &QTimer::timeout, this, &psa_van_receiver::get_vin_data);
    connect(this->status_timer, &QTimer::timeout, this, &psa_van_receiver::get_status_data);
    connect(this->preset_timer, &QTimer::timeout, this, &psa_van_receiver::get_preset_data);

    connect(this->engine_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::engine_data_received);
    connect(this->radio_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::radio_data_received);
    connect(this->trip_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::trip_data_received);
    connect(this->time_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::time_data_received);
    connect(this->headunit_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::headunit_data_received);
    connect(this->cd_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::cd_data_received);
    connect(this->door_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::door_data_received);
    connect(this->dash_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::dash_data_received);
    connect(this->vin_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::vin_data_received);
    connect(this->status_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::status_data_received);
    connect(this->preset_data_future_watcher, &QFutureWatcher<void>::finished, this, &psa_van_receiver::preset_data_received);

}

void psa_van_receiver::receive_engine_packets(bool receive, int msec){
    if(this->engine_data == NULL)
    {
        this->engine_data = new psa_engine_data_t;
        explicit_bzero(this->engine_data, sizeof(psa_engine_data_t));
    }

    if(this->new_engine_data == NULL)
    {
        this->new_engine_data = new psa_engine_data_t;
        explicit_bzero(this->new_engine_data, sizeof(psa_engine_data_t));
    }
    if(receive)
    {
        engine_timer->stop();
        engine_timer->start(msec);
        this->receiving_engine_data = true;
//        if(this->receiving_engine_data == false)
//        {
//            engine_timer->start(msec);
//        }
    }
    else{
        engine_timer->stop();
        this->receiving_engine_data = false;
    }
}

void psa_van_receiver::receive_radio_packets(bool receive, int msec)
{
    if(this->radio_data == NULL)
    {
        this->radio_data = new psa_radio_data_t;
        explicit_bzero(this->radio_data, sizeof(psa_radio_data_t));
    }

    if(this->new_radio_data == NULL)
    {
        this->new_radio_data = new psa_radio_data_t;
        explicit_bzero(this->new_radio_data, sizeof(psa_radio_data_t));
    }
    if(receive)
    {
        radio_timer->stop();
        radio_timer->start(msec);
        this->receiving_radio_data = true;
    }
    else{
        radio_timer->stop();
        this->receiving_radio_data = false;
    }
}

void psa_van_receiver::receive_preset_packet(bool receive, int msec)
{
    if(this->preset_data == NULL)
    {
        this->preset_data = new psa_preset_data_t;
        explicit_bzero(this->preset_data, sizeof(psa_preset_data_t));
    }

    if(this->new_preset_data == NULL)
    {
        this->new_preset_data = new psa_preset_data_t;
        explicit_bzero(this->new_preset_data, sizeof(psa_preset_data_t));
    }
    if(receive)
    {
        preset_timer->stop();
        preset_timer->start(msec);
        this->receiving_preset_data = true;
    }
    else{
        preset_timer->stop();
        this->receiving_preset_data = false;
    }
}

void psa_van_receiver::receive_trip_packets(bool receive, int msec)
{
    if(this->trip_data == NULL)
    {
        this->trip_data = new psa_trip_data_t;
        explicit_bzero(this->trip_data, sizeof(psa_trip_data_t));
    }

    if(this->new_trip_data == NULL)
    {
        this->new_trip_data = new psa_trip_data_t;
        explicit_bzero(this->new_trip_data, sizeof(psa_trip_data_t));
    }
    if(receive)
    {
        trip_timer->stop();
        trip_timer->start(msec);
        this->receiving_trip_data = true;
    }
    else{
        trip_timer->stop();
        this->receiving_trip_data = false;
    }
}

void psa_van_receiver::receive_time_packets(bool receive, int msec)
{
    if(this->time_data == NULL)
    {
        this->time_data = new psa_time_data_t;
        explicit_bzero(this->time_data, sizeof(psa_time_data_t));
    }

    if(this->new_time_data == NULL)
    {
        this->new_time_data = new psa_time_data_t;
        explicit_bzero(this->new_time_data, sizeof(psa_time_data_t));
    }
    if(receive)
    {
        time_timer->stop();
        time_timer->start(msec);
        this->receiving_time_data = true;
    }
    else{
        time_timer->stop();
        this->receiving_time_data = false;
    }
}

void psa_van_receiver::receive_headunit_packets(bool receive, int msec)
{
    if(this->headunit_data == NULL)
    {
        this->headunit_data = new psa_headunit_data_t;
        explicit_bzero(this->headunit_data, sizeof(psa_headunit_data_t));
    }

    if(this->new_headunit_data == NULL)
    {
        this->new_headunit_data = new psa_headunit_data_t;
        explicit_bzero(this->new_headunit_data, sizeof(psa_headunit_data_t));
    }
    if(receive)
    {
        headunit_timer->stop();
        headunit_timer->start(msec);
        this->receiving_headunit_data = true;
    }
    else{
        headunit_timer->stop();
        this->receiving_headunit_data = false;
    }
}

void psa_van_receiver::receive_cd_player_packets(bool receive, int msec)
{
    if(this->cd_data == NULL)
    {
        this->cd_data = new psa_cd_player_data_t;
        explicit_bzero(this->cd_data, sizeof(psa_cd_player_data_t));
    }

    if(this->new_cd_data == NULL)
    {
        this->new_cd_data = new psa_cd_player_data_t;
        explicit_bzero(this->new_cd_data, sizeof(psa_cd_player_data_t));
    }
    if(receive)
    {
        cd_timer->stop();
        cd_timer->start(msec);
        this->receiving_cd_data = true;
    }
    else{
        cd_timer->stop();
        this->receiving_cd_data = false;
    }
}

void psa_van_receiver::receive_door_packets(bool receive, int msec)
{
    if(this->door_data == NULL)
    {
        this->door_data = new psa_door_data_t;
        explicit_bzero(this->door_data, sizeof(psa_door_data_t));
    }

    if(this->new_door_data == NULL)
    {
        this->new_door_data = new psa_door_data_t;
        explicit_bzero(this->new_door_data, sizeof(psa_door_data_t));
    }
    if(receive)
    {
        door_timer->stop();
        door_timer->start(msec);
        this->receiving_door_data = true;
    }
    else{
        door_timer->stop();
        this->receiving_door_data = false;
    }
}

void psa_van_receiver::receive_dash_packets(bool receive, int msec)
{
    if(this->dash_data == NULL)
    {
        this->dash_data = new psa_dash_data_t;
        explicit_bzero(this->dash_data, sizeof(psa_dash_data_t));
    }

    if(this->new_dash_data == NULL)
    {
        this->new_dash_data = new psa_dash_data_t;
        explicit_bzero(this->new_dash_data, sizeof(psa_dash_data_t));
    }
    if(receive)
    {
        dash_timer->stop();
        dash_timer->start(msec);
        this->receiving_dash_data = true;
    }
    else{
        dash_timer->stop();
        this->receiving_dash_data = false;
    }
}

void psa_van_receiver::receive_vin_packets(bool receive, int msec)
{
    if(this->vin_data == NULL)
    {
        this->vin_data = new psa_vin_data_t;
        explicit_bzero(this->vin_data, sizeof(psa_vin_data_t));
    }

    if(this->new_vin_data == NULL)
    {
        this->new_vin_data = new psa_vin_data_t;
        explicit_bzero(this->new_vin_data, sizeof(psa_vin_data_t));
    }
    if(receive)
    {
        vin_timer->stop();
        vin_timer->start(msec);
        this->receiving_vin_data = true;
    }
    else{
        vin_timer->stop();
        this->receiving_vin_data = false;
    }
}

void psa_van_receiver::receive_status_packet(bool receive, int msec)
{
    if(this->status_data == NULL)
    {
        this->status_data = new psa_status_data_t;
        explicit_bzero(this->status_data, sizeof(psa_status_data_t));
    }

    if(this->new_status_data == NULL)
    {
        this->new_status_data = new psa_status_data_t;
        explicit_bzero(this->new_status_data, sizeof(psa_status_data_t));
    }
    if(receive)
    {
        status_timer->stop();
        status_timer->start(msec);
        this->receiving_status_data = true;
    }
    else{
        status_timer->stop();
        this->receiving_status_data = false;
    }
}

void psa_van_receiver::engine_data_received()
{
    emit engine_data_changed(*this->new_engine_data);
    return;
}

void psa_van_receiver::get_engine_data()
{
    QFuture<void> engine_future = QtConcurrent::run(this, &psa_van_receiver::queue_engine_data_async);
    this->engine_data_future_watcher->setFuture(engine_future);
}

void psa_van_receiver::queue_engine_data_async()
{
    //struct psa_engine_data new_data;

    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_engine_data(this->new_engine_data);
    locker.unlock();
}

void psa_van_receiver::radio_data_received()
{
    emit radio_data_changed(*this->new_radio_data);
    return;
}

void psa_van_receiver::get_radio_data()
{
    QFuture<void> radio_future = QtConcurrent::run(this, &psa_van_receiver::queue_radio_data_async);
    this->radio_data_future_watcher->setFuture(radio_future);
}

void psa_van_receiver::queue_radio_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_radio_data(this->new_radio_data);
    locker.unlock();
}

void psa_van_receiver::preset_data_received()
{
    emit preset_data_changed(*this->new_preset_data);
    return;
}

void psa_van_receiver::get_preset_data()
{
    QFuture<void> preset_future = QtConcurrent::run(this, &psa_van_receiver::queue_preset_data_async);
    this->preset_data_future_watcher->setFuture(preset_future);
}

void psa_van_receiver::queue_preset_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_radio_preset_data(this->new_preset_data);
    locker.unlock();
}


void psa_van_receiver::trip_data_received()
{
    emit trip_data_changed(*this->new_trip_data);
    return;
}

void psa_van_receiver::get_trip_data()
{
    QFuture<void> trip_future = QtConcurrent::run(this, &psa_van_receiver::queue_trip_data_async);
    this->trip_data_future_watcher->setFuture(trip_future);
}

void psa_van_receiver::queue_trip_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_trip_data(this->new_trip_data);
    locker.unlock();
}

void psa_van_receiver::time_data_received()
{
    emit time_data_changed(*this->new_time_data);
    return;
}

void psa_van_receiver::get_time_data()
{
    QFuture<void> time_future = QtConcurrent::run(this, &psa_van_receiver::queue_time_data_async);
    this->time_data_future_watcher->setFuture(time_future);
}

void psa_van_receiver::queue_time_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_time(this->new_time_data);
    locker.unlock();
}

void psa_van_receiver::headunit_data_received()
{
    emit headunit_data_changed(*this->new_headunit_data);
    return;
}

void psa_van_receiver::get_headunit_data()
{
    QFuture<void> headunit_future = QtConcurrent::run(this, &psa_van_receiver::queue_headunit_data_async);
    this->headunit_data_future_watcher->setFuture(headunit_future);
}

void psa_van_receiver::queue_headunit_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_headunit_data(this->new_headunit_data);
    locker.unlock();
}

void psa_van_receiver::cd_data_received()
{
    emit cd_player_data_changed(*this->new_cd_data);
    return;
}

void psa_van_receiver::get_cd_data()
{
    QFuture<void> cd_future = QtConcurrent::run(this, &psa_van_receiver::queue_cd_data_async);
    this->cd_data_future_watcher->setFuture(cd_future);
}

void psa_van_receiver::queue_cd_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_cd_player_data(this->new_cd_data);
    locker.unlock();
}

void psa_van_receiver::door_data_received()
{
    emit door_data_changed(*this->new_door_data);
    return;
}

void psa_van_receiver::get_door_data()
{
    QFuture<void> door_future = QtConcurrent::run(this, &psa_van_receiver::queue_door_data_async);
    this->door_data_future_watcher->setFuture(door_future);
}

void psa_van_receiver::queue_door_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_door_data(this->new_door_data);
    locker.unlock();
}

void psa_van_receiver::dash_data_received()
{
    emit dash_data_changed(*this->new_dash_data);
    return;
}

void psa_van_receiver::get_dash_data()
{
    QFuture<void> dash_future = QtConcurrent::run(this, &psa_van_receiver::queue_dash_data_async);
    this->dash_data_future_watcher->setFuture(dash_future);
}

void psa_van_receiver::queue_dash_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_dash_data(this->new_dash_data);
    locker.unlock();
}

void psa_van_receiver::status_data_received()
{
    emit status_data_changed(*this->new_status_data);
    return;
}

void psa_van_receiver::get_status_data()
{
    QFuture<void> status_future = QtConcurrent::run(this, &psa_van_receiver::queue_status_data_async);
    this->status_data_future_watcher->setFuture(status_future);
}

void psa_van_receiver::queue_status_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_car_status(this->new_status_data);
    locker.unlock();
}



void psa_van_receiver::vin_data_received()
{
    emit vin_changed(*this->new_vin_data);
    return;
}

void psa_van_receiver::get_vin_data()
{
    QFuture<void> vin_future = QtConcurrent::run(this, &psa_van_receiver::queue_vin_data_async);
    this->vin_data_future_watcher->setFuture(vin_future);
}

void psa_van_receiver::queue_vin_data_async()
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_get_vin(this->new_vin_data);
    locker.unlock();
}

void psa_van_receiver::queue_cdc_packet_async(psa_cd_changer_data_t cdc_data)
{
    QMutexLocker locker(&mutex);
    locker.relock();
    psa_set_cd_changer_data(&cdc_data);
    locker.unlock();
}

void psa_van_receiver::send_cdc_data(psa_cd_changer_data_t cdc_data)
{
    QtConcurrent::run(this, &psa_van_receiver::queue_cdc_packet_async, cdc_data);
    return;
}

psa_van_receiver::~psa_van_receiver()
{
    if(this->engine_data)
    {
        delete this->engine_data;
        this->engine_data = NULL;
    }
    if(this->radio_data)
    {
        delete this->radio_data;
        this->radio_data = NULL;
    }
    if(this->trip_data)
    {
        delete this->trip_data;
        this->trip_data = NULL;
    }
    if(this->door_data)
    {
        delete this->door_data;
        this->door_data = NULL;
    }
    if(this->dash_data)
    {
        delete this->dash_data;
        this->dash_data = NULL;
    }
    if(this->vin_data)
    {
        delete this->vin_data;
        this->vin_data = NULL;
    }
    if(this->time_data)
    {
        delete this->time_data;
        this->time_data = NULL;
    }
    if(this->headunit_data)
    {
        delete this->headunit_data;
        this->headunit_data = NULL;
    }
    if(this->cd_data)
    {
        delete this->cd_data;
        this->cd_data = NULL;
    }
    if(this->status_data)
    {
        delete this->status_data;
        this->status_data = NULL;
    }

    psa_close();
}
