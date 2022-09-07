#include "psa_music_window.h"
#include "ui_psa_music_window.h"
#include "libpsa/header/psa/psa_packet_defs.h"
#include "libpsa/header/psa/psa_receive.h"

#include <QMediaMetaData>
#include <QAudioDeviceInfo>
#include <QPixmap>
#include <QFile>
#include <QBuffer>
#include <QThread>
#include <QtConcurrent/qtconcurrentrun.h>
#include <iostream>
#include <QDir>
#include <QListWidgetItem>
#include <QGraphicsProxyWidget>
#include <QStateMachine>
#include <QState>
#include <QSignalTransition>
#include <QPropertyAnimation>
#include <QMediaPlaylist>
#include <QScroller>
#include <QBitmap>
#include <QPainter>
#include <QTimer>
#include <QDebug>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#define PSA_DEFAULT_VOLUME 100

QDir music_directory;
QDir root_music_directory;


QStateMachine* folder_list_statemachine;
QState* folder_list_state_open;
QState* folder_list_state_closed;

QStateMachine* music_list_statemachine;
QState* music_list_state_open;
QState* music_list_state_closed;

QStringList music_file_list;


QSignalTransition *open_closed;
QSignalTransition *open_closed_music;
QSignalTransition *closed_open;

QString title;
QString artist;
QPixmap *album_art;
QString album;

QStringList media_name_list;

static psa_cd_changer_data_t cdc_data;
static psa_cd_changer_data_t old_cdc_data;
static int total_duration;

static int global_volume;
static int music_loaded = 0;
static int auto_play = 0;
static int playlist_loaded = 0;

psa_music_window::psa_music_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::psa_music_window)
{
    ui->setupUi(this);
    this->metadata_future_watcher = new QFutureWatcher<void>;
    this->media_playlist = new QMediaPlaylist(this);
    this->player = new QMediaPlayer();
    this->playlist_tags_future_watcher = new QFutureWatcher<void>;
    album_art = new QPixmap;

    ui->volume_label->hide();

    this->cdc_data_timer = new QTimer(this);
    this->usb_refresh_timer = new QTimer (this);
    connect(this->cdc_data_timer, &QTimer::timeout, this, &psa_music_window::send_cdc_packet);
    connect(this->usb_refresh_timer, &QTimer::timeout, this, &psa_music_window::refresh_music_directory_usb);
//    std::cout<<this->ui->play_pause_button->styleSheet().toStdString()<<std::endl;
    cdc_data.cd_number = 1;
    cdc_data.cds_present = 0x01;
    cdc_data.track_number = 0;
    cdc_data.total_tracks = 0;
    cdc_data.track_time_minutes = 0;
    cdc_data.track_time_seconds = 0;
    cdc_data.status = PSA_CD_CHANGER_BUSY;

    connect(player, &QMediaPlayer::mediaStatusChanged, this, &psa_music_window::audio_player_media_status_chaged);
    connect(player, &QMediaPlayer::stateChanged, this, &psa_music_window::audio_player_state_changed);
    connect(player, QOverload<>::of(&QMediaObject::metaDataChanged), this, &psa_music_window::audio_metadata_changed);
    connect(player, &QMediaPlayer::durationChanged, this, &psa_music_window::music_duration_changed);
    connect(player, &QMediaPlayer::positionChanged, this, &psa_music_window::music_position_changed);

    connect(this->media_playlist, &QMediaPlaylist::currentIndexChanged, this, &psa_music_window::playlist_index_changed);

    connect(this->metadata_future_watcher, &QFutureWatcher<void>::finished, this, &psa_music_window::display_new_metadata);
    connect(this->playlist_tags_future_watcher, &QFutureWatcher<void>::finished, this, &psa_music_window::playlist_tags_ready);

    connect(ui->play_pause_button, &QPushButton::clicked, this, &psa_music_window::play_pause);
    connect(ui->volume_slider, &QSlider::valueChanged, this, &psa_music_window::change_volume);

    connect(ui->next_button, &QPushButton::clicked, this, &psa_music_window::next_track);
    connect(ui->prev_button, &QPushButton::clicked, this, &psa_music_window::prev_track);

    connect(ui->stop_button, &QPushButton::clicked, this, &psa_music_window::stop_player);

    connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &psa_music_window::handle_error);

    connect(ui->playlist_list, &QListWidget::itemClicked, this, &psa_music_window::playlist_list_item_selected);

    connect(ui->music_list, &QListWidget::itemClicked, this, &psa_music_window::folder_list_item_selected);

    player->setAudioRole(QAudio::MusicRole);
//    this->ui->music_list.scro
    this->ui->music_list->setAttribute(Qt::WA_AcceptTouchEvents);
    this->ui->music_list->move(0,400);
    QScroller::grabGesture(this->ui->music_list, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(this->ui->playlist_list, QScroller::LeftMouseButtonGesture);

//    root_music_directory.setPath("/home/mive/Dokumenti/music_test");
    root_music_directory.setPath("/run/media");

    if(!root_music_directory.exists())
    {
        root_music_directory.mkpath(".");
    }
//    root_music_directory.setPath("/home/root");
    music_directory.setPath(root_music_directory.path());


    folder_list_statemachine = new QStateMachine(this);
    folder_list_state_closed = new QState(folder_list_statemachine);
    folder_list_state_open = new QState(folder_list_statemachine);
    folder_list_statemachine->setInitialState(folder_list_state_closed);

    folder_list_state_closed->assignProperty(this->ui->folder_button, "checked", false);
    folder_list_state_closed->assignProperty(this->ui->music_list, "geometry", QRect(120,400,560,400));
    folder_list_state_open->assignProperty(this->ui->folder_button, "checked", true);
    folder_list_state_open->assignProperty(this->ui->music_list, "geometry", QRect(120,0,560,400));

    closed_open = folder_list_state_closed->addTransition(this->ui->folder_button, &QPushButton::clicked, folder_list_state_open);
    open_closed = folder_list_state_open->addTransition(this->ui->folder_button, &QPushButton::clicked, folder_list_state_closed);

    open_closed_music = folder_list_state_open->addTransition(this, &psa_music_window::media_loaded, folder_list_state_closed);

    music_list_statemachine = new QStateMachine(this);
    music_list_state_closed = new QState(music_list_statemachine);
    music_list_state_open = new QState(music_list_statemachine);
    music_list_statemachine->setInitialState(music_list_state_closed);

    music_list_state_closed->assignProperty(this->ui->playlist_button, "checked", false);
    music_list_state_closed->assignProperty(this->ui->playlist_list, "geometry", QRect(0,400,680,400));
    music_list_state_open->assignProperty(this->ui->playlist_button, "checked", true);
    music_list_state_open->assignProperty(this->ui->playlist_list, "geometry", QRect(0,0,680,400));

    closed_open = music_list_state_closed->addTransition(this->ui->playlist_button, &QPushButton::clicked, music_list_state_open);
    open_closed = music_list_state_open->addTransition(this->ui->playlist_button, &QPushButton::clicked, music_list_state_closed);

    open_closed_music = music_list_state_open->addTransition(this, &psa_music_window::media_loaded, music_list_state_closed);


//    closed_open->addAnimation(new QPropertyAnimation(w, "rotation"));
    closed_open->addAnimation(new QPropertyAnimation(this->ui->music_list, "geometry"));
//    open_closed->addAnimation(new QPropertyAnimation(w, "rotation"));
    open_closed->addAnimation(new QPropertyAnimation(this->ui->music_list, "geometry"));

//    open_closed_music->addAnimation(new QPropertyAnimation(w, "rotation"));
    open_closed_music->addAnimation(new QPropertyAnimation(this->ui->music_list, "geometry"));

    folder_list_statemachine->start();
    music_list_statemachine->start();

    this->music_dir_watcher = new QFileSystemWatcher(this);
    this->music_dir_watcher->addPath(root_music_directory.absolutePath());

    this->usb_device_watcher = new QFileSystemWatcher(this);
    //this->usb_device_watcher->addPath("/dev");

    connect(this->music_dir_watcher, &QFileSystemWatcher::directoryChanged, this, &psa_music_window::refresh_root_music_directory);
    connect(this->usb_device_watcher, &QFileSystemWatcher::directoryChanged, this, &psa_music_window::new_device_connected);
    refresh_root_music_directory();
    this->reset_player();
    global_volume = PSA_DEFAULT_VOLUME;
    player->setVolume(global_volume);
    this->ui->volume_slider->setValue(global_volume);
    this->ui->volume_slider->hide();
    this->ui->volume_label->setText(QString::asprintf("Volume: %d", global_volume));

    cdc_data_timer->setSingleShot(true);
    cdc_data_timer->start(2000);
}

void psa_music_window::set_van_handle(psa_van_receiver* van_handle)
{
    this->van_receiver = van_handle;
}

void psa_music_window::send_cdc_packet()
{
    if(this->van_receiver != NULL)
    {
        if(memcmp(&cdc_data, &old_cdc_data, sizeof(cdc_data)))
        {
            this->van_receiver->send_cdc_data(cdc_data);
            old_cdc_data = cdc_data;
        }
    }
    //QtConcurrent::run(this, &psa_music_window::queue_cdc_packet_async);
}

void psa_music_window::queue_cdc_packet_async()
{
    psa_set_cd_changer_data(&cdc_data);
}

void psa_music_window::reset_player()
{
    //player->stop();
    this->media_playlist->clear();
    this->ui->playlist_list->clear();
    player->setMedia(NULL);
    music_loaded = 0;

    cdc_data.status = PSA_CD_CHANGER_BUSY;
    cdc_data.track_number = 0xff;
    cdc_data.total_tracks = 0xff;
    cdc_data.track_time_minutes = 0xff;
    cdc_data.track_time_seconds = 0xff;
    this->send_cdc_packet();
    this->ui->duration_label->setText("00:00 / 00:00");
    this->ui->music_cover->setPixmap(QPixmap(":/images/icons/cover-placeholder.png"));
    this->ui->music_album->setText("");
    this->ui->music_artist->setText("");
    this->ui->music_title->setText("");
}

void psa_music_window::stop_player()
{
    this->reset_player();
    music_directory.setPath(root_music_directory.absolutePath());
    //this->reset_player();
    this->refresh_music_directory(false);
}

void psa_music_window::pause_player()
{
    cdc_data.status = PSA_CD_CHANGER_PLAYING;
    this->send_cdc_packet();
    player->pause();
}

void psa_music_window::resume_player()
{
    if(music_loaded)
    {
        cdc_data.status = PSA_CD_CHANGER_PLAYING;
        this->send_cdc_packet();
        player->play();
    }
}

void psa_music_window::next_track()
{
    if(music_loaded)
    {
        this->media_playlist->next();
    }
}

void psa_music_window::prev_track()
{
    if(music_loaded)
    {
        this->media_playlist->previous();
    }
}

/**
 * @brief psa_music_window::refresh_root_music_directory.
 *
 * Refreshes `root_music_directory`, resets the player, and sets `music_directory` to `root_music_directory`.
 * It's called every time something in `root_music_directory` changes.
 */
void psa_music_window::refresh_root_music_directory()
{
    std::cout<<"Refreshing root directory"<<std::endl;

    this->reset_player();

    music_directory.setPath(root_music_directory.absolutePath());

    QStringList root_folder_list = root_music_directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if(!root_folder_list.isEmpty()) // Checkout the drive, if it's got only music on it, load it
    {
        music_directory.cd(root_folder_list[0]);
        this->usb_refresh_timer->setSingleShot(true);
        this->usb_refresh_timer->start(3000);
    }
    else{
        this->refresh_music_directory(false);

    }

}

void psa_music_window::refresh_music_directory_usb()
{
    this->refresh_music_directory(true);
}

/**
 * @brief psa_music_window::refresh_music_directory.
 * Lists the folders under current `music_directory`.
 * If the folder doesn't have any child folders, all the audio files inside are loaded as a playlist.
 */
void psa_music_window::refresh_music_directory(bool auto_load)
{
    music_directory.refresh();

    QStringList music_folder_list;

    if(music_directory.path() == root_music_directory.path())
    {
        music_folder_list = music_directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot); // Get all child folders

        if (music_folder_list.empty() && auto_load == true) // If there are no child folders,
        {
            this->load_playlist_directory();
        }
    }
    else{
        music_folder_list = music_directory.entryList(QDir::Dirs | QDir::NoDot); // Get all child folders

        if (music_folder_list.size() <= 1 && auto_load == true) // If there are no child folders,
        {
            this->load_playlist_directory();
        }
    }

    ui->music_list->clear();
    ui->music_list->addItems(music_folder_list);
}

void psa_music_window::new_device_connected(const QString &directory)
{
    std::cout<<directory.toStdString()<<std::endl;
    music_directory.setPath(directory);
    refresh_music_directory(true);

    this->usb_device_watcher->removePath(directory);
}

void psa_music_window::load_playlist_directory()
{
    this->reset_player();
    this->ui->playlist_list->clear();

    music_file_list = music_directory.entryList(QStringList() << "*.mp3" << "*.opus" << "*.ogg" << "*.m4a" << "*.wma" << "*.aac" << "*.webm", QDir::Files, QDir::Name);

    QList<QMediaContent> media_content_list;


    // Create a playlist with all music files in the folder

    for(int i = 0; i < music_file_list.size(); ++i)
    {
        media_content_list.append(QUrl::fromLocalFile(music_directory.filePath(music_file_list[i])));
    }

    this->media_playlist->clear();
    this->media_playlist->addMedia(media_content_list);

    //Shuffle the playlist
    this->media_playlist->shuffle();
    this->media_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    player->setPlaylist(this->media_playlist);

    this->ui->playlist_list->clear();
    this->ui->playlist_list->addItem("Loading...");
    playlist_loaded = 0;

    QFuture<void> f = QtConcurrent::run(this, &psa_music_window::get_playlist_tags);
    this->playlist_tags_future_watcher->setFuture(f);

    //player->pause();
    music_loaded = 1;

//    music_directory.cd("..");
//    refresh_music_directory(false);

    cdc_data.total_tracks = this->media_playlist->mediaCount();
    cdc_data.track_number = 1;

    emit media_loaded();


}



void psa_music_window::get_playlist_tags()
{
    //this->reset_player();

    media_name_list.clear();


    // Insert the Shuffled playlist into a QListWidget

    for(int i = 0; i < this->media_playlist->mediaCount(); ++i)
    {
        QString curr_media_path = this->media_playlist->media(i).request().url().path();

        TagLib::FileRef f(curr_media_path.toStdString().c_str());

        if(!f.isNull() && f.tag())
        {
            TagLib::Tag *tag = f.tag();

            QString track_name = QString::fromStdString(tag->artist().to8Bit(true)) + " - " + QString::fromStdString(tag->title().to8Bit(true));

            media_name_list.append(track_name);

        }
        else{
            QFileInfo info(curr_media_path);
            media_name_list.append(info.fileName());
        }
    }



}


void psa_music_window::playlist_tags_ready()
{

    this->ui->playlist_list->clear();
    this->ui->playlist_list->addItems(media_name_list);
    playlist_loaded = 1;
}

void psa_music_window::playlist_list_item_selected(QListWidgetItem *item)
{
    if(playlist_loaded == 0)
    {
        return;
    }
    int index = ui->playlist_list->row(item);

    //player->stop();

    music_loaded = 1;
    this->media_playlist->setCurrentIndex(index);
//    std::cout<<music_file_list[index].toStdString()<<std::endl;

//    QString file_path = music_directory.filePath(music_file_list[index]);

//    if(QFile::exists(file_path))
//    {
//        emit media_loaded();
//        this->load_media(index);
//    }

    emit media_loaded();
}

void psa_music_window::playlist_index_changed(int index)
{
    cdc_data.track_number = index +1;
    this->send_cdc_packet();
    this->ui->playlist_list->setCurrentRow(index, QItemSelectionModel::SelectCurrent);
}

void psa_music_window::folder_list_item_selected(QListWidgetItem *item)
{
    auto_play = 1;
    QString new_folder = item->text();
    std::cout<<new_folder.toStdString()<<std::endl;

    music_directory.cd(new_folder);
    refresh_music_directory(true);

}

void psa_music_window::load_media(int index)
{
    player->stop();
    music_loaded = 1;
    player->setPlaylist(this->media_playlist);
    this->media_playlist->setCurrentIndex(index);

}


void psa_music_window::audio_metadata_changed()
{
    if(music_loaded)
    {
        QFuture<void> metadata_future = QtConcurrent::run(this, &psa_music_window::load_new_metadata);
        this->metadata_future_watcher->setFuture(metadata_future);
    }
}



void psa_music_window::load_new_metadata()
{
    QFontMetrics title_metrics(ui->music_title->font());
    QFontMetrics artist_metrics(ui->music_title->font());
    QFontMetrics album_metrics(ui->music_title->font());


    QString temp_title = player->metaData("Title").toString();
    if (temp_title.isNull())
    {
        QString curr_media_path = this->media_playlist->currentMedia().request().url().path();
        QFileInfo file_info(curr_media_path);
        title = file_info.fileName();
    }
    else{
        title = temp_title;

    }

    title = title_metrics.elidedText(title, Qt::ElideRight, ui->music_title->width());
    //title = temp_title;
    artist = (player->metaData(QMediaMetaData::AlbumArtist)).toString();
    artist = artist_metrics.elidedText(artist, Qt::ElideRight, ui->music_artist->width());
    //QPixmap temp_art = QPixmap::fromImage(player->metaData("CoverArtImage").value<QImage>());
    //*album_art = temp_art.scaled(240,240, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QImage new_album_art_image = player->metaData("CoverArtImage").value<QImage>();


    if(new_album_art_image.isNull())
    {
        *album_art = QPixmap(":/images/icons/cover-placeholder.png");
    }
    else{

        // Rounded corners


        QImage album_art_image = new_album_art_image.scaled(240,240, Qt::KeepAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
        QImage rounded_album_art = QImage(album_art_image.width(), album_art_image.height(), QImage::Format_ARGB32);
        rounded_album_art.fill(Qt::transparent);
        QBrush brush = QBrush(album_art_image);
        QPainter painter(&rounded_album_art);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(0,0,rounded_album_art.width(),rounded_album_art.height(), 20,20);
        painter.end();

        *album_art = QPixmap::fromImage(rounded_album_art);
    }

    album = player->metaData("AlbumTitle").toString();

    album = album_metrics.elidedText(album, Qt::ElideRight, ui->music_album->width());
}

void psa_music_window::display_new_metadata()
{
    ui->music_title->setText(title);
    ui->music_artist->setText(artist);
    ui->music_album->setText(album);
    ui->music_cover->setPixmap(*album_art);
}

void psa_music_window::music_duration_changed(qint64 duration)
{
    total_duration = duration / 1000;
}

void psa_music_window::music_position_changed(qint64 position)
{

    int total_seconds = position / 1000;

    cdc_data.track_time_seconds = total_seconds % 60;
    cdc_data.track_time_minutes = total_seconds / 60;
    cdc_data.track_number = this->media_playlist->currentIndex() +1;
    cdc_data.total_tracks = this->media_playlist->mediaCount();
    this->send_cdc_packet();
    this->ui->duration_label->setText(QString::asprintf("%02d:%02d / %02d:%02d", total_seconds / 60, total_seconds % 60, total_duration / 60, total_duration % 60));
}

void psa_music_window::change_volume(int volume)
{
    player->setVolume(volume);
    global_volume = volume;
    this->ui->volume_label->setText(QString::asprintf("Volume: %d", global_volume));
}

void psa_music_window::handle_error(QMediaPlayer::Error error)
{
    fprintf(stderr, "Error %d\n", error);
    fflush(stderr);
}

void psa_music_window::audio_player_media_status_chaged(QMediaPlayer::MediaStatus status)
{
    if(status == QMediaPlayer::LoadedMedia)
    {
        music_loaded = 1;
        printf("Loaded media\n");
        fflush(stdout);
        player->setVolume(global_volume);

        if(auto_play)
        {
            player->play();
            auto_play = 0;
        }

        //player->play();
    }

//    switch(status)
//    {
//        case QMediaPlayer::LoadedMedia:
//            ui->play_pause_button->setEnabled(true);
//        break;
//        default:
//            ui->play_pause_button->setEnabled(false);
//        break;
//    }
}

void psa_music_window::play_pause(bool checked)
{
    if(player->state() == QMediaPlayer::PlayingState)
    {
        if(!checked)
        {
            //QtConcurrent::run(player, &QMediaPlayer::pause);
            player->pause();
        }
    }
    else{
        if(checked)
        {
            //QtConcurrent::run(player, &QMediaPlayer::play);
            player->play();
        }
    }
}

void psa_music_window::audio_player_state_changed(QMediaPlayer::State state)
{
    switch(state)
    {
        case QMediaPlayer::StoppedState:
        case QMediaPlayer::PausedState:
        default:
            cdc_data.status = PSA_CD_CHANGER_PLAYING;
            this->send_cdc_packet();
            ui->play_pause_button->setChecked(false);
        break;
        case QMediaPlayer::PlayingState:
            cdc_data.status = PSA_CD_CHANGER_PLAYING;
            this->send_cdc_packet();
            ui->play_pause_button->setChecked(true);
        break;
    }
}

psa_music_window::~psa_music_window()
{
    delete ui;
}
