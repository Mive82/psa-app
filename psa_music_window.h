#ifndef PSA_MUSIC_WINDOW_H
#define PSA_MUSIC_WINDOW_H

#include "psa_van_receiver.h"
#include "qgraphicsscene.h"
#include <QWidget>
#include <QOpenGLWidget>
#include <QMediaPlayer>
#include <QThread>
#include <QListWidget>
#include <QFileSystemWatcher>
#include <QFutureWatcher>

namespace Ui {
class psa_music_window;
}

class psa_music_window : public QWidget
{
    Q_OBJECT

public:
    explicit psa_music_window(QWidget *parent = nullptr);
    ~psa_music_window();

public slots:

    void audio_metadata_changed();
    //void audio_available_changed(bool available);

    //void audio_duration_changed(qint64 duration);
    void audio_player_media_status_chaged(QMediaPlayer::MediaStatus status);

    void audio_player_state_changed(QMediaPlayer::State state);

    void play_pause(bool checked);

    void change_volume(int volume);

    void handle_error(QMediaPlayer::Error error);

    void refresh_root_music_directory();

    void refresh_music_directory_usb();

    void refresh_music_directory(bool auto_load=false);

    void get_playlist_tags();

    void playlist_tags_ready();

    void load_playlist_directory();

    void load_media(int index);

    void playlist_list_item_selected(QListWidgetItem* item);

    void playlist_index_changed(int index);

    void folder_list_item_selected(QListWidgetItem* item);

    void music_position_changed(qint64 position);

    void music_duration_changed(qint64 duration);

    void display_new_metadata();

    void new_device_connected(const QString &directory);

    void send_cdc_packet();

    void set_van_handle(psa_van_receiver* van_handle);

    void next_track();

    void prev_track();

    /**
     * @brief stop_player
     * Stops the player, and closes all file handles on the usb drive.
     * Call this before unplugging the drive.
     */
    void stop_player();

    void pause_player();

    void resume_player();

signals:

    void media_loaded();


private:
    Ui::psa_music_window *ui;
    QTimer *cdc_data_timer, *usb_refresh_timer;
    QMediaPlayer *player;
    QMediaPlaylist *media_playlist;
    QFileSystemWatcher *music_dir_watcher, *usb_device_watcher;
    QFutureWatcher<void> *metadata_future_watcher, *playlist_tags_future_watcher;
    psa_van_receiver *van_receiver = NULL;
    void reset_player();
    void load_new_metadata();
    void queue_cdc_packet_async();
    //bool playing;

};

#endif // PSA_MUSIC_WINDOW_H
