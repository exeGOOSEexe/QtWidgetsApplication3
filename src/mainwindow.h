#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMediaPlayer>
#include "audioplayer.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <map>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onPlayClicked();
    void onTrackDoubleClicked(QListWidgetItem* item);
	void onTracksReceived(QNetworkReply* reply);
	void onNextClicked();
	void onPrevClicked();
    void onPauseClicked();

    void updatePosition(qint64 position);
    void setPosition(int position);
    void setDuration(qint64 duration);

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
    QPushButton* m_btnNext;
	QPushButton* m_btnPause;

    QNetworkAccessManager* m_networkManager;
    std::map<QListWidgetItem*, QString> m_trackUrls;
};