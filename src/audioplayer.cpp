extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
#include <libavutil/audio_fifo.h> 
}
#include "audioplayer.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <vector>
#include <algorithm>
#include <QDebug>

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent)
{
	avformat_network_init();

	m_stopFlag = false;

	QAudioDevice device = QMediaDevices::defaultAudioOutput();

	QAudioFormat format;
	format.setSampleRate(44100);
	format.setChannelCount(2);
	format.setSampleFormat(QAudioFormat::Int16);

	m_audioSink = new QAudioSink(device, format, this);
	m_audioDevice = m_audioSink->start();
	// Добавь это в конструктор AudioPlayer (после m_audioDevice = m_audioSink->start();)
	connect(this, &AudioPlayer::audioDataReady, this, [this](const QByteArray& data) {
		if (m_audioDevice) {
			m_audioDevice->write(data);
		}
		}, Qt::DirectConnection);
}

AudioPlayer::~AudioPlayer()
{
	stop();

	avformat_network_deinit();
}

void AudioPlayer::play(std::string url)
{
	stop();
	m_stopFlag = false;

	m_decodingThread = std::thread(&AudioPlayer::decodingLoop, this, url);
}

void AudioPlayer::stop()
{
	m_stopFlag = true;
	if (m_decodingThread.joinable()) m_decodingThread.join();
}

void AudioPlayer::pause()
{
	m_audioSink->suspend();
}

void AudioPlayer::resume()
{
	m_audioSink->resume();
}

void AudioPlayer::next() {}
void AudioPlayer::before() {}

void AudioPlayer::setVolume(int volume)
{
	if (m_audioSink) {
		// Переводим шкалу 0-100 в 0.0-1.0
		m_audioSink->setVolume(volume / 100.0);
	}
}

void AudioPlayer::setPosition(int64_t ms)
{
	m_seekTarget = ms;
}

void AudioPlayer::decodingLoop(std::string url)
{
	AVFormatContext* s = avformat_alloc_context();
	int ret = avformat_open_input(&s, url.c_str(), NULL, NULL);

	if (ret < 0)
	{
		char errbuf[128];
		av_strerror(ret, errbuf, sizeof(errbuf));
		avformat_free_context(s);
		return;
	}

	int stream_info = avformat_find_stream_info(s, NULL);
	if (stream_info < 0)
	{
		avformat_free_context(s);
		return;
	}

	int index = av_find_best_stream(s, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (index < 0)
	{
		avformat_free_context(s);
		return;
	}

	AVStream* pStream = s->streams[index];

	// Отправляем длину трека в интерфейс
	if (s->duration != AV_NOPTS_VALUE) {
		int64_t duration_ms = s->duration / 1000;
		emit durationChanged(duration_ms);
	}
	m_seekTarget = -1; // Сбрасываем цель перемотки

	AVCodecParameters* pCodecParameters = pStream->codecpar;
	const AVCodec* pCodec = avcodec_find_decoder(pCodecParameters->codec_id);

	AVCodecContext* pCodecContext = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(pCodecContext, pStream->codecpar);
	int codec_open = avcodec_open2(pCodecContext, pCodec, NULL);

	if (codec_open < 0)
	{
		avcodec_free_context(&pCodecContext);
		avformat_free_context(s);
		return;
	}

	AVChannelLayout out_ch_layout;
	av_channel_layout_default(&out_ch_layout, 2);
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	int out_sample_rate = 44100;

	SwrContext* swr = NULL;
	swr_alloc_set_opts2(&swr,
		&out_ch_layout,
		out_sample_fmt,
		out_sample_rate,
		&pStream->codecpar->ch_layout,
		(AVSampleFormat)pStream->codecpar->format,
		pStream->codecpar->sample_rate,
		0,
		NULL
	);
	swr_init(swr);

	AVPacket* packet = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();

	AVAudioFifo* fifo = av_audio_fifo_alloc(out_sample_fmt, out_ch_layout.nb_channels, 1);
	AVFrame* resampled_frame = av_frame_alloc();

	while (!m_stopFlag)
	{
		// 1. Проверка перемотки
		int64_t targetMs = m_seekTarget.exchange(-1);
		if (targetMs != -1) {
			int64_t targetTs = targetMs * 1000; // Переводим мс в микросекунды для FFmpeg
			av_seek_frame(s, -1, targetTs, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(pCodecContext);
			av_audio_fifo_reset(fifo);
		}

		// 2. Чтение кадра
		if (av_read_frame(s, packet) < 0) {
			break; // Конец файла
		}

		if (packet->stream_index != index) {
			av_packet_unref(packet);
			continue;
		}

		int send_packet = avcodec_send_packet(pCodecContext, packet);
		if (send_packet < 0)
		{
			if (send_packet != AVERROR(EAGAIN)) break;
		}

		while ((send_packet = avcodec_receive_frame(pCodecContext, frame)) == 0)
		{
			// Отправляем текущую позицию в интерфейс
			if (frame->pts != AV_NOPTS_VALUE) {
				int64_t posMs = frame->pts * av_q2d(pStream->time_base) * 1000;
				emit positionChanged(posMs);
			}

			// ИСПРАВЛЕНИЕ: Заново задаем параметры после unref
			resampled_frame->sample_rate = out_sample_rate;
			resampled_frame->ch_layout = out_ch_layout;
			resampled_frame->format = out_sample_fmt;

			int conv = swr_convert_frame(swr, resampled_frame, frame);
			av_audio_fifo_write(fifo, (void**)resampled_frame->data, resampled_frame->nb_samples);

			av_frame_unref(resampled_frame);
			av_frame_unref(frame);

			while (av_audio_fifo_size(fifo) > 0 && !m_stopFlag)
			{
				int bytes_free = m_audioSink->bytesFree();

				if (bytes_free > 0) {
					int samplesToRead = std::min(bytes_free / 4, av_audio_fifo_size(fifo)); // 4 байта = 1 сэмпл
					if (samplesToRead > 0) {
						int bytesToRead = samplesToRead * 4;
						std::vector<char> buffer(bytesToRead);
						char* bufPtr = buffer.data();

						av_audio_fifo_read(fifo, (void**)&bufPtr, samplesToRead);
						QByteArray outData(buffer.data(), bytesToRead);
						emit audioDataReady(outData);
					}
				}
				else {
					// Буфер полон, ждем 10 мс
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}

				if (av_audio_fifo_size(fifo) < 22050) break;
			}
		}
		av_packet_unref(packet);
	}

	av_packet_free(&packet);
	av_frame_free(&resampled_frame);
	av_frame_free(&frame);
	swr_free(&swr);
	av_audio_fifo_free(fifo);
	avcodec_free_context(&pCodecContext);
	avformat_close_input(&s);
}