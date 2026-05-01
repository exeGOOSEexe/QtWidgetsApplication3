#include "mainwindow.h"
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_player = new AudioPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    setupUi();
    applyDarkTheme();
    // Подключаем кнопки к функциям
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_playlist, &QListWidget::itemDoubleClicked, this, &MainWindow::onTrackDoubleClicked);
	m_networkManager = new QNetworkAccessManager(this);
	connect(m_networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onTracksReceived);
    QNetworkRequest request(QUrl("http://2.26.67.101:8000/tracks"));
	m_networkManager->get(request);
}

MainWindow::~MainWindow() {
}

void MainWindow::onTracksReceived(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        m_lblCurrentTrack->setText("Ошибка сети: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    // Читаем ответ сервера
    QByteArray responseData = reply->readAll();

    // Парсим JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isArray()) {
        m_lblCurrentTrack->setText("Ошибка: сервер вернул не массив");
        reply->deleteLater();
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    m_playlist->clear(); // Очищаем список перед загрузкой
    m_trackUrls.clear(); // Очищаем карту ссылок

    // Перебираем все треки из базы данных
    for (int i = 0; i < jsonArray.size(); ++i) {
        QJsonObject trackObj = jsonArray[i].toObject();

        // Вытаскиваем данные (согласно моделям FastAPI)
        QString title = trackObj["title"].toString();
        QString author = trackObj["author"].toString();
        QString fileUrl = trackObj["file_url"].toString();

        // Создаем красивую строчку для списка: "Автор - Название"
        QString displayString = author + " - " + title;
        QListWidgetItem* item = new QListWidgetItem(displayString, m_playlist);

        // Сохраняем реальную ссылку на MP3 в словарь (чтобы потом передать в плеер)
        m_trackUrls[item] = fileUrl;
    }

    if (m_playlist->count() > 0) {
        m_lblCurrentTrack->setText("Треки успешно загружены!");
    }
    else {
        m_lblCurrentTrack->setText("На сервере пока нет треков");
    }

    reply->deleteLater(); // Очищаем память
}

void MainWindow::onPlayClicked() {
    QListWidgetItem* item = m_playlist->currentItem();
    if (item) {
        QString url = item->text();
        m_lblCurrentTrack->setText("Играет: " + url);
        m_player->play(url.toStdString());
        
        m_btnPlay->hide();
        m_btnStop->show();
    }
}

void MainWindow::onStopClicked() {
    m_player->stop();
    m_lblCurrentTrack->setText("Остановлено");
    
    m_btnStop->hide();
    m_btnPlay->show();
}

void MainWindow::onNextClicked() {
    // Здесь можно добавить код для перехода к следующему треку
}

void MainWindow::onPrevClicked() {
    // Здесь можно добавить код для перехода к предыдущему треку
}

void MainWindow::setVolume(int volume) {
    m_audioOutput->setVolume(volume / 100.0);
}

void MainWindow::updatePosition(qint64 position) {
    // Обновляем ползунок только если пользователь не перетаскивает его вручную
    if (!m_sliderProgress->isSliderDown()) {
        m_sliderProgress->setValue(static_cast<int>(position));
    }
}

void MainWindow::setDuration(qint64 duration) {
    m_sliderProgress->setRange(0, static_cast<int>(duration));
}

void MainWindow::setPosition(int position) {
 
}

void MainWindow::onTrackDoubleClicked(QListWidgetItem* item) {
    // Достаем ссылку на MP3-файл, привязанную к этому элементу списка
    QString url = m_trackUrls[item];

    m_lblCurrentTrack->setText("Играет: " + item->text());
    m_player->play(url.toStdString());
}

void MainWindow::setupUi() {
    this->setWindowTitle("AudioStreamPlayer");
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
    // Подключаем перетаскивание ползунка к перемотке трека
    connect(m_sliderProgress, &QSlider::sliderMoved, this, &MainWindow::setPosition);
    mainLayout->addWidget(m_sliderProgress);

    // 4. Панель кнопок (горизонтальный слой)
    QHBoxLayout* controlsLayout = new QHBoxLayout();

    m_btnPrev = new QPushButton("⏮", this);
    m_btnPlay = new QPushButton("▶ Play", this);
    m_btnStop = new QPushButton("⏹ Stop", this);
    m_btnNext = new QPushButton("⏭", this);
    m_btnStop->hide(); // Скрываем кнопку стоп изначально

    controlsLayout->addStretch(); // Сдвигает кнопки в центр
    controlsLayout->addWidget(m_btnPrev);
    controlsLayout->addWidget(m_btnPlay);
    controlsLayout->addWidget(m_btnStop);
    controlsLayout->addWidget(m_btnNext);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);

    // 5. Ползунок громкости (в отдельном слое)
    QHBoxLayout* volLayout = new QHBoxLayout();
    
    volLayout->addWidget(new QLabel("🔊", this));
    m_sliderVolume = new QSlider(Qt::Horizontal, this);
    m_sliderVolume->setRange(0, 100);
    m_sliderVolume->setValue(70); // Синхронизируем начальное значение ползунка
    m_sliderVolume->setPageStep(1);
    m_audioOutput->setVolume(0.7);
    m_sliderVolume->setMaximumWidth(100); // Делаем его коротким
    // Подключаем изменение значения ползунка к изменению громкости
    volLayout->addWidget(m_sliderVolume);

    mainLayout->addLayout(volLayout);

    // 1. Связываем изменение ползунка громкости с плеером
    connect(m_sliderVolume, &QSlider::valueChanged, m_player, &AudioPlayer::setVolume);

    // 2. При загрузке трека устанавливаем максимум ползунка прогресса (длину трека)
    connect(m_player, &AudioPlayer::durationChanged, this, [this](int64_t duration) {
        m_sliderProgress->setMaximum(duration);
        });

    // 3. Во время воспроизведения ползунок ползет вперед
    connect(m_player, &AudioPlayer::positionChanged, this, [this](int64_t pos) {
        // Важно: двигаем ползунок программно, ТОЛЬКО если пользователь сам его сейчас не тащит мышкой
        if (!m_sliderProgress->isSliderDown()) {
            m_sliderProgress->blockSignals(true); // Блокируем сигналы, чтобы не зациклить перемотку
            m_sliderProgress->setValue(pos);
            m_sliderProgress->blockSignals(false);
        }
        });

  
    connect(m_sliderProgress, &QSlider::sliderReleased, this, [this]() {
        m_player->setPosition(m_sliderProgress->value());
        });
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