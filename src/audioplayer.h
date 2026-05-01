#pragma once

#include <QObject>
#include <QAudioSink>
#include <QIODevice>
#include <string>
#include <thread>
#include <atomic>

class AudioPlayer : public QObject {
	Q_OBJECT

public:
	explicit AudioPlayer(QObject* parent = nullptr);
	~AudioPlayer();

	void play(std::string url);
	void stop();
	void pause();
	void resume();
	void next();
	void before();

	// Новые методы для управления из интерфейса
	void setVolume(int volume);
	void setPosition(int64_t ms);

signals:
	// Сигналы для передачи времени в интерфейс (ползунок прогресса)
	void durationChanged(int64_t duration_ms);
	void positionChanged(int64_t position_ms);
	void audioDataReady(const QByteArray& data);
private:
	void decodingLoop(std::string url);

	QAudioSink* m_audioSink = nullptr;
	QIODevice* m_audioDevice = nullptr;

	std::atomic<bool> m_stopFlag;
	std::thread m_decodingThread;

	// Переменная для безопасной перемотки между потоками (-1 значит перемотка не нужна)
	std::atomic<int64_t> m_seekTarget{ -1 };
};