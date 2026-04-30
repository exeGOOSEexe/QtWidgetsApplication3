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

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent)
{
	m_stopFlag = false;

	QAudioDevice device = QMediaDevices::defaultAudioOutput();

	// 2. Настраиваем формат (например, 44.1кГц, Стерео, Float)
	QAudioFormat format;
	format.setSampleRate(44100);
	format.setChannelCount(2);
	format.setSampleFormat(QAudioFormat::Int16); // Или Float

	m_audioSink = new QAudioSink(device, format, this);
	m_audioDevice = m_audioSink->start();
}

AudioPlayer::~AudioPlayer()
{
	stop();
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
void AudioPlayer::next()
{
	// как можно реализовать эту функцию?
	// Ответ: 
}
void AudioPlayer::before()
{

}
void AudioPlayer::decodingLoop(std::string url)
{
	AVFormatContext* s = avformat_alloc_context();
	int ret = avformat_open_input(&s, url.c_str(), NULL, NULL);

	if (ret < 0)
	{
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
	
	AVCodecParameters* pCodecParameters = s->streams[index]->codecpar;
	const AVCodec* pCodec = avcodec_find_decoder(pCodecParameters->codec_id);

	AVStream* pStream = s->streams[index];
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
	resampled_frame->sample_rate = out_sample_rate;
	resampled_frame->ch_layout = out_ch_layout;
	resampled_frame->format = out_sample_fmt;

	while (av_read_frame(s, packet) == 0 && !m_stopFlag)
	{
		if (packet->stream_index != index) { av_packet_unref(packet); continue; }
		int send_packet = avcodec_send_packet(pCodecContext, packet);
		if (send_packet < 0)
		{
			if (send_packet != AVERROR(EAGAIN)) break;
		}

		while ((send_packet = avcodec_receive_frame(pCodecContext, frame)) == 0)
		{
			int conv = swr_convert_frame(swr, resampled_frame, frame);
			av_audio_fifo_write(fifo, (void**)resampled_frame->data, resampled_frame->nb_samples);
			av_frame_unref(resampled_frame);
			av_frame_unref(frame);

			while (av_audio_fifo_size(fifo) > 0)
			{
				int bytes_free = m_audioSink->bytesFree();

				if (bytes_free > 0) {
					int samplesToRead = std::min(bytes_free / 4, av_audio_fifo_size(fifo)); // 4 байта = 1 сэмпл
					if (samplesToRead > 0) {
						int bytesToRead = samplesToRead * 4;
						std::vector<char> buffer(bytesToRead);
						char* bufPtr = buffer.data();

						av_audio_fifo_read(fifo, (void**)&bufPtr, samplesToRead);
						m_audioDevice->write(buffer.data(), bytesToRead);
					}
				}
				else {
					// Колонкам нужно время проиграть звук, спим 10 миллисекунд
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
