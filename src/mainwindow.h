#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "audioplayer.h" // Подключаем наш движок

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onTrackDoubleClicked(QListWidgetItem* item);

private:
    void setupUi();
    void applyDarkTheme();

    AudioPlayer* m_player; // Наш движок воспроизведения

    // Элементы интерфейса
    QListWidget* m_playlist;
    QLabel* m_lblCurrentTrack;
    QSlider* m_sliderProgress;
    QSlider* m_sliderVolume;

    QPushButton* m_btnPrev;
    QPushButton* m_btnPlay;
    QPushButton* m_btnPause;
    QPushButton* m_btnStop;
    QPushButton* m_btnNext;
};