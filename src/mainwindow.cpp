#include "mainwindow.h"
#include <QWidget>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QFileInfo>
#include <QStandardPaths>
#include <QRegularExpression>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_player = new AudioPlayer(this);
    setupUi();
    applyDarkTheme();

    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(m_playlist, &QListWidget::itemDoubleClicked, this, &MainWindow::onTrackDoubleClicked);
    connect(m_btnPause, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(m_btnNext, &QPushButton::clicked, this, &MainWindow::onNextClicked);
    connect(m_btnPrev, &QPushButton::clicked, this, &MainWindow::onPrevClicked);
    connect(m_btnDownload, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(m_btnLogin, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_btnUpload, &QPushButton::clicked, this, &MainWindow::onUploadClicked);

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

// =========================================================
//                  СЕТЕВАЯ АВТОРИЗАЦИЯ
// =========================================================

void MainWindow::onLoginClicked() {
    if (!m_authToken.isEmpty()) {
        m_authToken.clear();
        m_btnLogin->setText("🔑 Войти");
        m_btnUpload->setEnabled(false);
        QMessageBox::information(this, "Выход", "Вы успешно вышли из аккаунта.");
        return;
    }
    bool ok;
    // Запрашиваем логин
    QString username = QInputDialog::getText(this, "Вход", "Введите логин:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;

    // Запрашиваем пароль
    QString password = QInputDialog::getText(this, "Вход", "Введите пароль:", QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;

    // Создаем отдельный менеджер сети для логина
    QNetworkAccessManager* authManager = new QNetworkAccessManager(this);
    connect(authManager, &QNetworkAccessManager::finished, this, &MainWindow::onLoginFinished);
    connect(authManager, &QNetworkAccessManager::finished, authManager, &QObject::deleteLater);

    // Подготавливаем запрос (FastAPI ждет данные в формате формы, а не JSON)
    QNetworkRequest request(QUrl("http://2.26.67.101:8000/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("username", username);
    postData.addQueryItem("password", password);

    // Отправляем POST запрос
    authManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void MainWindow::onLoginFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль!");
        reply->deleteLater();
        return;
    }

    // Читаем токен из ответа
    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("access_token")) {
        m_authToken = jsonObj["access_token"].toString();

        QMessageBox::information(this, "Успех", "Вы успешно авторизовались!");

        // ИЗМЕНЕНО: Не блокируем кнопку, а меняем текст, чтобы можно было выйти!
        m_btnLogin->setText("🚪 Выйти");
        m_btnLogin->setEnabled(true);
        m_btnUpload->setEnabled(true);
    }
    reply->deleteLater();
}

// =========================================================
//                  ЗАГРУЗКА ТРЕКА (UPLOAD)
// =========================================================

void MainWindow::onUploadClicked() {
    if (m_authToken.isEmpty()) return;

    // 1. Выбираем файл
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите MP3 файл", "", "Audio Files (*.mp3)");
    if (filePath.isEmpty()) return;

    // 2. Спрашиваем автора и название
    bool ok;
    QString author = QInputDialog::getText(this, "Информация о треке", "Кто автор?", QLineEdit::Normal, "", &ok);
    if (!ok || author.isEmpty()) return;

    QString title = QInputDialog::getText(this, "Информация о треке", "Как называется трек?", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    // 3. Открываем файл для чтения
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл на компьютере!");
        delete file;
        return;
    }

    // 4. Формируем "MultiPart" (составной) запрос, чтобы отправить и текст, и файл
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart authorPart;
    authorPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"author\""));
    authorPart.setBody(author.toUtf8());
    multiPart->append(authorPart);

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
    titlePart.setBody(title.toUtf8());
    multiPart->append(titlePart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(QFileInfo(filePath).fileName())));
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // Файл удалится вместе с multiPart
    multiPart->append(filePart);

    // 5. Отправляем на сервер
    QNetworkRequest request(QUrl("http://2.26.67.101:8000/tracks/upload"));
    // ВАЖНО: Добавляем JWT токен в заголовок, иначе сервер нас прогонит (401 Unauthorized)
    request.setRawHeader("Authorization", QString("Bearer " + m_authToken).toUtf8());

    QNetworkAccessManager* uploadManager = new QNetworkAccessManager(this);
    QNetworkReply* reply = uploadManager->post(request, multiPart);
    multiPart->setParent(reply); // Чтобы память очистилась сама

    // 6. Обрабатываем ответ (через лямбда-выражение, чтобы не писать отдельную функцию)
    connect(uploadManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep) {
        if (rep->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Успех", "Трек успешно загружен на сервер!");

            // Запрашиваем новый список треков, чтобы новая песня сразу появилась в плейлисте!
            QNetworkRequest req(QUrl("http://2.26.67.101:8000/tracks"));
            m_networkManager->get(req);
        }
        else {
            QMessageBox::warning(this, "Ошибка сервера", "Не удалось загрузить трек: " + rep->errorString());
        }
        rep->deleteLater();
        uploadManager->deleteLater();
        });
}
// =========================================================
//                  СКАЧИВАНИЕ ТРЕКА
// =========================================================

void MainWindow::onDownloadClicked() {
    QListWidgetItem* item = m_playlist->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите трек в списке для скачивания!");
        return;
    }

    // 1. Формируем красивое имя файла и путь по умолчанию (Папка "Музыка")
    QString url = m_trackUrls[item];
    QString safeFileName = item->text().replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_") + ".mp3";
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/" + safeFileName;

    // 2. Спрашиваем пользователя, куда сохранить
    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить трек как...", defaultPath, "Audio Files (*.mp3)");
    if (savePath.isEmpty()) return;

    // 3. Начинаем скачивание
    QNetworkAccessManager* downloadManager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QNetworkReply* reply = downloadManager->get(request);

    // Показываем пользователю, что процесс пошел (можно вывести в текст играющего трека)
    m_lblCurrentTrack->setText("Скачивание: " + safeFileName + " ...");

    // 4. Ждем окончания скачивания
    connect(downloadManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep) {
        if (rep->error() == QNetworkReply::NoError) {
            // Сохраняем байты в файл
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(rep->readAll());
                file.close();
                QMessageBox::information(this, "Успех", "Трек успешно сохранен!");
                m_lblCurrentTrack->setText("Скачивание завершено!");
            }
            else {
                QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл на диск (нет прав).");
            }
        }
        else {
            QMessageBox::warning(this, "Ошибка сети", "Не удалось скачать трек: " + rep->errorString());
        }

        rep->deleteLater();
        downloadManager->deleteLater();
        });
}
void MainWindow::onPlayClicked() {
    QListWidgetItem* item = m_playlist->currentItem();
    if (!item) return; // Если ни один трек не выделен, ничего не делаем

    // Проверяем: если кнопка Stop видна (значит плеер не остановлен)
    // И выделенный трек совпадает с тем, что написано на экране — это снятие с паузы
    if (m_lblCurrentTrack->text() == "Играет: " + item->text()) {
        m_player->resume();
    }
    else {
        // Иначе (это новый трек или мы нажимали Stop) — запускаем с нуля
        QString url = m_trackUrls[item];
        m_lblCurrentTrack->setText("Играет: " + item->text());
        m_player->play(url.toStdString());
    }

    m_btnPlay->hide();
    m_btnPause->show();

}

void MainWindow::onPauseClicked() {
    m_player->pause();
    m_btnPause->hide();
    m_btnPlay->show(); // Показываем Play для возобновления
}

void MainWindow::onTrackDoubleClicked(QListWidgetItem* item) {
    // Выделяем элемент, чтобы onPlayClicked знал, что играть
    m_playlist->setCurrentItem(item);

    // Просто вызываем логику кнопки Play, чтобы не дублировать код
    onPlayClicked();
}

void MainWindow::onNextClicked() {
    int count = m_playlist->count();
    if (count == 0) return; // Если плейлист пуст, ничего не делаем

    int currentRow = m_playlist->currentRow();

    // Вычисляем следующий индекс. Если дошли до конца, перепрыгиваем в начало (0)
    int nextRow = (currentRow + 1) % count;

    m_playlist->setCurrentRow(nextRow);
    onPlayClicked(); // Сразу начинаем играть
}

void MainWindow::onPrevClicked() {
    int count = m_playlist->count();
    if (count == 0) return;

    int currentRow = m_playlist->currentRow();

    // Вычисляем предыдущий индекс. Если мы на первом треке, перепрыгиваем в самый конец
    int prevRow = (currentRow - 1 + count) % count;

    m_playlist->setCurrentRow(prevRow);
    onPlayClicked(); // Сразу начинаем играть
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

void MainWindow::setupUi() {
    this->setWindowTitle("AudioStreamPlayer");
    this->resize(600, 400);

    // Главный виджет, который займет всё окно
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Основной вертикальный слой
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* topMenuLayout = new QHBoxLayout();
    m_btnLogin = new QPushButton("🔑 Войти", this);
    m_btnUpload = new QPushButton("⬆ Загрузить трек", this);
    m_btnDownload = new QPushButton("⬇ Скачать", this); // НОВАЯ КНОПКА
    m_btnUpload->setEnabled(false);

    topMenuLayout->addWidget(m_btnLogin);
    topMenuLayout->addWidget(m_btnUpload);
    topMenuLayout->addWidget(m_btnDownload); // ДОБАВЛЯЕМ В СЛОЙ
    topMenuLayout->addStretch();

    mainLayout->addLayout(topMenuLayout);

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
    m_btnPause = new QPushButton("⏸ Pause", this);
    m_btnNext = new QPushButton("⏭", this);

    m_btnPause->hide();

    controlsLayout->addStretch(); // Сдвигает кнопки в центр
    controlsLayout->addWidget(m_btnPrev);
    controlsLayout->addWidget(m_btnPlay);
    controlsLayout->addWidget(m_btnPause);
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
    m_player->setVolume(70);
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