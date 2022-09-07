#include "psa_radio_window.h"
#include "qicon.h"
#include "ui_psa_radio_window.h"

#include <QPixmap>
#include <iostream>

static int clock_state = 0;
static int old_signal_str = 0;

static enum psa_radio_source radio_source = PSA_RADIO_NONE;

static QPixmap* signal_0, *signal_1, *signal_2, *signal_3, *signal_4;

psa_radio_window::psa_radio_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::psa_radio_window)
{
    ui->setupUi(this);
    this->preset_labels.resize(6);
    this->preset_labels[0] = ui->preset_1;
    this->preset_labels[1] = ui->preset_2;
    this->preset_labels[2] = ui->preset_3;
    this->preset_labels[3] = ui->preset_4;
    this->preset_labels[4] = ui->preset_5;
    this->preset_labels[5] = ui->preset_6;

    this->preset_list.resize(6);
    for(int i = 0; i< 6;++i)
    {
        this->preset_list[i] = "";
    }

    this->ui->big_clock_label->hide();
    this->ui->big_date_label->hide();

    this->ui->curr_band->hide();
    this->ui->presets_window->hide();
    this->ui->radio_freq->hide();
    this->ui->station_name->hide();
    this->ui->signal_icon->hide();

    this->ui->cd_icon->hide();
    this->ui->cd_tracks->hide();
    this->clock_timer = new QTimer(this);


    signal_0 = new QPixmap(QIcon(":/symbols/icons/signal-0.svg").pixmap(QSize(37,37)));
    signal_1 = new QPixmap(QIcon(":/symbols/icons/signal-1.svg").pixmap(QSize(37,37)));
    signal_2 = new QPixmap(QIcon(":/symbols/icons/signal-2.svg").pixmap(QSize(37,37)));
    signal_3 = new QPixmap(QIcon(":/symbols/icons/signal-3.svg").pixmap(QSize(37,37)));
    signal_4 = new QPixmap(QIcon(":/symbols/icons/signal-4.svg").pixmap(QSize(37,37)));

    this->ui->signal_icon->setPixmap(*signal_0);

    //connect(this->clock_timer, &QTimer::timeout, this, &psa_radio_window::change_clock_state);
    //this->clock_timer->start(1000);

}

void psa_radio_window::receive_radio_data(psa_radio_data_t radio_data)
{
    if(radio_source == PSA_RADIO_TUNER)
    {

        QString stat_name = QString::fromLocal8Bit(radio_data.station);
        change_station(stat_name);


    //#ifdef PSA_SIMULATE
        change_freq((float)radio_data.freq/100);
    //#else
      //  change_freq((float)radio_data.freq * 0.05 + 50);
    //#endif
        change_preset(radio_data.preset);

        change_band((enum psa_radio_band)radio_data.band);

        uint8_t new_signal_str = radio_data.signal_strength;

        if(new_signal_str != old_signal_str)
        {
            switch(new_signal_str)
            {
            default:
            case 0:
            case 1:
            case 2:
                this->ui->signal_icon->setPixmap(*signal_0);
                break;
            case 3:
            case 4:
            case 5:
                this->ui->signal_icon->setPixmap(*signal_1);
                break;
            case 6:
            case 7:
            case 8:
                this->ui->signal_icon->setPixmap(*signal_2);
                break;
            case 9:
            case 10:
            case 11:
                this->ui->signal_icon->setPixmap(*signal_3);
                break;
            case 12:
            case 13:
            case 14:
            case 15:
                this->ui->signal_icon->setPixmap(*signal_4);
                break;
            }
            old_signal_str = new_signal_str;
        }
    }
}

void psa_radio_window::receive_cd_player_data(psa_cd_player_data_t cd_data)
{
    if(radio_source == PSA_RADIO_INTERNAL)
    {
        if(cd_data.status & PSA_CD_PLAYER_ERROR)
        {
            this->ui->cd_tracks->setText("CD Error!");
        }
        else if(cd_data.status & PSA_CD_PLAYER_LOADING)
        {
            this->ui->cd_tracks->setText("CD Loading...");
        }
        else if(cd_data.status & PSA_CD_PLAYER_EJECTING)
        {
            this->ui->cd_tracks->setText("Ejecting...");
        }
        else{
            this->ui->radio_freq->setText(QString::asprintf("%02d:%02d", cd_data.current_minute, cd_data.current_second));
            this->ui->cd_tracks->setText(QString::asprintf("Track %d/%d", cd_data.current_track, cd_data.total_tracks));
        }
    }
}

void psa_radio_window::change_clock_state()
{
    if(clock_state)
    {
        clock_state = 0;
    }
    else{
        clock_state = 1;
    }
}

void psa_radio_window::receive_time_data(psa_time_data_t time_data)
{
    if(time_data.valid_time)
    {
        this->ui->big_clock_label->setText(QString::asprintf("%02d:%02d:%02d", time_data.hour, time_data.minutes, time_data.seconds));
        this->ui->big_date_label->setText(QString::asprintf("%02d. %02d. %4d", time_data.day, time_data.month + 1, time_data.year + 1900));

    }
    else{
        this->ui->big_clock_label->setText("");
        this->ui->big_date_label->setText("");
    }
}

void psa_radio_window::receive_headunit_data(psa_headunit_data_t headunit_data)
{
    if(headunit_data.unit_powered_on == 0)
    {
        change_source(PSA_RADIO_NONE);
    }
    else{
        change_source((enum psa_radio_source)headunit_data.source);
    }
    if(headunit_data.cd_present)
    {
        ui->cd_icon->show();
    }
    else{
        ui->cd_icon->hide();
    }

}

void psa_radio_window::change_band(enum psa_radio_band band)
{
    // <html><head/><body><p><span style=" font-size:14pt;">FM 2</span></p></body></html>

    if(radio_source == PSA_RADIO_TUNER)
    {
        switch(band)
        {
            case PSA_AM_1:
                ui->curr_band->setText("AM 1");
            break;
            case PSA_AM_2:
                ui->curr_band->setText("AM 2");
            break;
            case PSA_AM_3:
                ui->curr_band->setText("AM 3");
            break;
            case PSA_FM_1:
                ui->curr_band->setText("FM 1");
            break;
            case PSA_FM_2:
                ui->curr_band->setText("FM 2");
            break;
            case PSA_FM_3:
                ui->curr_band->setText("FM 3");
            break;
            case PSA_FM_AST:
                ui->curr_band->setText("FMast");
            break;
        }
    }
}

void psa_radio_window::change_station(QString name)
{

//    this->station_name = name;
//    name.prepend("<html><head/><body><p align=\"center\"><span style=\" font-size:55pt; font-weight:600;\">");
//    name.append("</span></p></body></html>");
    ui->station_name->setText(name);

}

void psa_radio_window::change_freq(float freq)
{
    ui->radio_freq->setText(QString::asprintf("%.2f", freq));
}

void psa_radio_window::change_radio_state(bool state)
{
    if(state)
    {
        this->ui->big_clock_label->hide();
        this->ui->big_date_label->hide();

        this->ui->curr_band->show();
        this->ui->presets_window->show();
        this->ui->radio_freq->show();
        this->ui->station_name->show();
    }
    else{
        this->ui->big_clock_label->show();
        this->ui->big_date_label->show();

        this->ui->curr_band->hide();
        this->ui->presets_window->hide();
        this->ui->radio_freq->hide();
        this->ui->station_name->hide();
    }
}

void psa_radio_window::change_source(enum psa_radio_source new_source)
{
    radio_source = new_source;

    switch(new_source)
    {
    case PSA_RADIO_NONE:
    default:

        change_station(QString::fromStdString("Radio OFF"));
        this->ui->big_clock_label->show();
        this->ui->big_date_label->show();
        this->ui->station_name->show();

        this->ui->curr_band->hide();
        this->ui->presets_window->hide();
        this->ui->radio_freq->hide();

        this->ui->signal_icon->hide();
        this->ui->cd_tracks->hide();
    break;
    case PSA_RADIO_TUNER:
        this->ui->big_clock_label->hide();
        this->ui->big_date_label->hide();
        this->ui->cd_tracks->hide();
        this->ui->signal_icon->show();

        this->ui->curr_band->show();
        this->ui->presets_window->show();
        this->ui->radio_freq->show();
        this->ui->station_name->show();

        break;
    case PSA_RADIO_INTERNAL:
        this->ui->big_clock_label->hide();
        this->ui->big_date_label->hide();
        this->ui->curr_band->hide();
        this->ui->presets_window->hide();

        this->ui->signal_icon->hide();
        this->ui->cd_tracks->show();
        this->ui->radio_freq->show();
        this->ui->station_name->show();

        change_station(QString::fromStdString("CD"));

        break;
    case PSA_RADIO_EXTERNAL:
        this->ui->big_clock_label->hide();
        this->ui->big_date_label->hide();
        this->ui->curr_band->hide();
        this->ui->presets_window->hide();

        this->ui->signal_icon->hide();
        this->ui->cd_tracks->hide();
        this->ui->radio_freq->hide();
        this->ui->station_name->show();

        change_station(QString::fromStdString("USB"));
        break;
    case PSA_RADIO_NAVIGATION:
        this->ui->big_clock_label->hide();
        this->ui->big_date_label->hide();
        this->ui->curr_band->hide();
        this->ui->presets_window->hide();
        this->ui->signal_icon->hide();

        this->ui->cd_tracks->hide();
        this->ui->radio_freq->hide();
        this->ui->station_name->show();

        change_station(QString::fromStdString("NAVI"));
        break;
    }
}

void psa_radio_window::change_preset(int preset)
{
    if(this->last_preset != 0)
    {
        preset_labels[last_preset-1]->setStyleSheet("QLabel{\n	margin: 5px;\n	color: rgb(255, 255, 255);\n}");
    }

    if(preset != 0)
    {
        preset_labels[preset-1]->setStyleSheet("QLabel{\n	margin: 5px;\n	border: 2px solid white;\nborder-radius: 10px;\n	color: rgb(255, 255, 255);\n}");
        preset_labels[preset-1]->setText((QString::asprintf("%d - ", preset)) + this->ui->station_name->text());
    }

    this->last_preset = preset;

}

void psa_radio_window::receive_presets_data(psa_preset_data_t preset_data)
{
    for(int i = 0; i< 6;++i)
    {
        QString temp_name = QString(preset_data.presets[i].preset_name);
        if(this->preset_list[i] != temp_name)
        {
            this->preset_list[i] = temp_name;
            preset_labels[i]->setText((QString::asprintf("%d - ", i+1)) + temp_name);
        }
    }
}

psa_radio_window::~psa_radio_window()
{
    delete ui;
}
