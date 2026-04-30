#pragma once

#include <QObject>
#include <QAudioSink>
#include <QIODevice>
#include <string>
#include <thread>
#include <atomic>

class AudioPlayer : public QObject
{
	Q_OBJECT
public:
	explicit AudioPlayer(QObject* parent = nullptr);
	~AudioPlayer();

	void play(std::string url);
	void stop();
	void pause();
	void next();
	void before();
	
private:
	void decodingLoop(std::string url);

	QAudioSink* m_audioSink = nullptr;
	QIODevice* m_audioDevice = nullptr;

	std::atomic<bool> m_stopFlag;
	std::thread m_decodingThread;
};