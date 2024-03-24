//
// Created by Qiu on 2024/3/24.
//
#ifndef FPCALC_FPCALC_READER_H
#define FPCALC_FPCALC_READER_H

#include "media/NdkMediaExtractor.h"
#include "media/NdkMediaCodec.h"
#include "media/NdkMediaFormat.h"

extern "C" {
#include "libavutil/opt.h"
#include "audio/ffmpeg_audio_processor_swresample.h"
};

enum LoopResultAction {
    CONTINUE, FINISHED, STOPPED
};

class MediaCodecReader {
public:
    ~MediaCodecReader() {
        Close();
    }

    bool Open(int fd);

    void Close();

    bool loopRead(std::function<LoopResultAction(const int16_t *data, size_t size)> callback);

    int GetSampleRate() const { return m_output_sample_rate; };

    int GetChannels() const { return m_output_channels; };

    double GetDuration() const { return duration_ms; };

    std::string GetError() const { return m_error; }

    bool SetInputFormat(char *input_fmt) {
        m_error = "No need to set this.";
        return false;
    };

    bool SetInputChannels(int input_channels) {
        m_input_channels = input_channels;
        return true;
    };

    bool SetInputSampleRate(int input_sample_rate) {
        m_input_sample_rate = input_sample_rate;
        return true;
    };

    void SetOutputSampleRate(int output_sample_rate) { m_output_sample_rate = output_sample_rate; }

    void SetOutputChannels(int output_channels) { m_output_channels = output_channels; }


private:
    AMediaExtractor *extractor_;
    AMediaFormat *format_;
    AMediaCodec *codec_;
    std::unique_ptr<chromaprint::FFmpegAudioProcessor> m_converter;

    int audio_track_index_ = -1;    // 解封装后选择的音轨的index
    double duration_ms = 0;        // 解码前获取到音频的时长(us微秒)
    int64_t duration_us = 0;        // 解码前获取到音频的时长(us微秒)
    int32_t channel_count = -1;     // 解码前获取到音频的声道数
    int32_t sample_rate = -1;       // 解码前获取到音频的采样率
    int m_output_sample_rate;       // 解码且重采样后的目标采样率
    int m_output_channels;          // 解码且重采样后的目标声道数
    int m_input_sample_rate;        // 用户指定的输入采样率
    int m_input_channels;           // 用户指定的输入声道数
    std::string m_error;
};

#endif //FPCALC_FPCALC_READER_H
