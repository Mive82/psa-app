#include "psa_badapple_window.h"
#include "ui_psa_badapple_window.h"
#include <QGraphicsVideoItem>
#include <QLayout>

psa_badapple_window::psa_badapple_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::psa_badapple_window)
{
    ui->setupUi(this);

    this->badapple_player = new QMediaPlayer(this);
    this->item = new QGraphicsVideoItem;
    this->badapple_player->setVideoOutput(this->ui->widget);
    //this->ui->graphicsView->scene()->addItem(item);
    this->ui->graphicsView->hide();


    this->badapple_player->setMedia(QUrl("qrc:/memes/memes/badapple.mp4"));
    this->badapple_player->setVolume(10);
}

void psa_badapple_window::start_playback()
{
    this->badapple_player->play();
}

void psa_badapple_window::stop_playback()
{
    this->badapple_player->pause();
    this->badapple_player->stop();
}





psa_badapple_window::~psa_badapple_window()
{
    delete ui;
}
