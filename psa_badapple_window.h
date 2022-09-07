#ifndef PSA_BADAPPLE_WINDOW_H
#define PSA_BADAPPLE_WINDOW_H

#include <QWidget>
#include <QMediaPlayer>
#include <QOpenGLWidget>
#include <QGraphicsVideoItem>

namespace Ui {
class psa_badapple_window;
}

class psa_badapple_window : public QWidget
{
    Q_OBJECT

public:
    explicit psa_badapple_window(QWidget *parent = nullptr);
    ~psa_badapple_window();

public slots:

    void start_playback();
    void stop_playback();

private:
    Ui::psa_badapple_window *ui;

    QGraphicsScene *scene;
    QOpenGLWidget* widget;
    QGraphicsVideoItem *item;
    QVideoWidget *video_widget;
    QMediaPlayer *badapple_player;

};

#endif // PSA_BADAPPLE_WINDOW_H
