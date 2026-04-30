#include "mainwindow.h"
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_player = new AudioPlayer(this);

    setupUi();
    applyDarkTheme();

    // Заполняем плейлист тестовыми данными (позже будешь брать их из JSON/БД)
    m_playlist->addItem("http://localhost:8000/music/01.mp3");
    m_playlist->addItem("http://localhost:8000/music/02.mp3");

    // Подключаем кнопки к функциям
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_playlist, &QListWidget::itemDoubleClicked, this, &MainWindow::onTrackDoubleClicked);
}

MainWindow::~MainWindow() {
    // m_player удалится автоматически, т.к. мы передали this в конструктор
}

void MainWindow::onPlayClicked() {
    QListWidgetItem* item = m_playlist->currentItem();
    if (item) {
        QString url = item->text();
        m_lblCurrentTrack->setText("Играет: " + url);
        m_player->play(url.toStdString());
    }
}

void MainWindow::onStopClicked() {
    m_player->stop();
    m_lblCurrentTrack->setText("Остановлено");
}

void MainWindow::onTrackDoubleClicked(QListWidgetItem* item) {
    QString url = item->text();
    m_lblCurrentTrack->setText("Играет: " + url);
    m_player->play(url.toStdString());
}

void MainWindow::setupUi() {
    this->setWindowTitle("Аудио Стриминг");
    this->resize(600, 400);

    // Главный виджет, который займет всё окно
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Основной вертикальный слой
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 1. Плейлист
    m_playlist = new QListWidget(this);
    mainLayout->addWidget(m_playlist);

    // 2. Текущий трек
    m_lblCurrentTrack = new QLabel("Выберите трек для воспроизведения", this);
    m_lblCurrentTrack->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_lblCurrentTrack);

    // 3. Ползунок прогресса (горизонтальный)
    m_sliderProgress = new QSlider(Qt::Horizontal, this);
    mainLayout->addWidget(m_sliderProgress);

    // 4. Панель кнопок (горизонтальный слой)
    QHBoxLayout* controlsLayout = new QHBoxLayout();

    m_btnPrev = new QPushButton("⏮", this);
    m_btnPlay = new QPushButton("▶ Play", this);
    m_btnPause = new QPushButton("⏸", this);
    m_btnStop = new QPushButton("⏹ Stop", this);
    m_btnNext = new QPushButton("⏭", this);

    controlsLayout->addStretch(); // Сдвигает кнопки в центр
    controlsLayout->addWidget(m_btnPrev);
    controlsLayout->addWidget(m_btnPlay);
    controlsLayout->addWidget(m_btnPause);
    controlsLayout->addWidget(m_btnStop);
    controlsLayout->addWidget(m_btnNext);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);

    // 5. Ползунок громкости (в отдельном слое)
    QHBoxLayout* volLayout = new QHBoxLayout();
    volLayout->addWidget(new QLabel("🔊", this));
    m_sliderVolume = new QSlider(Qt::Horizontal, this);
    m_sliderVolume->setRange(0, 100);
    m_sliderVolume->setValue(50);
    m_sliderVolume->setMaximumWidth(150); // Делаем его коротким
    volLayout->addWidget(m_sliderVolume);

    mainLayout->addLayout(volLayout);
}

void MainWindow::applyDarkTheme() {
    // Вся магия цветов (QSS)
    QString darkStyle = R"(
        QMainWindow {
            background-color: #121212;
        }
        QLabel {
            color: #E0E0E0;
            font-size: 14px;
            font-weight: bold;
        }
        QListWidget {
            background-color: #1E1E1E;
            color: #E0E0E0;
            border: 1px solid #333333;
            border-radius: 5px;
            padding: 5px;
            font-size: 14px;
        }
        QListWidget::item:selected {
            background-color: #007ACC;
            color: #FFFFFF;
            border-radius: 3px;
        }
        QListWidget::item:hover {
            background-color: #2D2D30;
        }
        QPushButton {
            background-color: #333333;
            color: #FFFFFF;
            border: none;
            border-radius: 5px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #444444;
        }
        QPushButton:pressed {
            background-color: #007ACC;
        }
        QSlider::groove:horizontal {
            border: 1px solid #333333;
            height: 6px;
            background: #1E1E1E;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #007ACC;
            border: none;
            width: 14px;
            margin: -4px 0;
            border-radius: 7px;
        }
        QSlider::handle:horizontal:hover {
            background: #0098FF;
        }
    )";

    this->setStyleSheet(darkStyle);
}