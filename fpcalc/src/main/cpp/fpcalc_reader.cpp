//
// Created by Qiu on 2024/3/23.
//

#include <unistd.h>
#include <sys/stat.h>
#include <jni.h>
#include <android/log.h>
#include <sstream>

#include "fpcalc_reader.h"

#define SetError(...)   __android_log_print((int)ANDROID_LOG_ERROR, "CHROMAPRINT", __VA_ARGS__)
#define LOGI(...)   __android_log_print((int)ANDROID_LOG_INFO, "CHROMAPRINT", __VA_ARGS__)

bool MediaCodecReader::Open(int fd) {
    extractor_ = AMediaExtractor_new();
    ssize_t length = lseek(fd, 0, SEEK_END);

    LOGI("file length %d", length);

    ssize_t ret = AMediaExtractor_setDataSourceFd(extractor_, fd, 0, length);
    if (ret < 0) {
        SetError("Error when set data source fd. return %d.", ret);
        extractor_ = nullptr;
        return false;
    }

    // 通过获取track的mimeType，获取音频track的index
    int track_cnt = AMediaExtractor_getTrackCount(extractor_);
    const char *info;
    for (int i = 0; i < track_cnt; i++) {
        format_ = AMediaExtractor_getTrackFormat(extractor_, i);
        AMediaFormat_getString(format_, AMEDIAFORMAT_KEY_MIME, &info);
        std::string str(info);
        if (str.find("audio") == 0) {
            AMediaExtractor_selectTrack(extractor_, i);
            audio_track_index_ = i;
            break;
        }
        AMediaFormat_delete(format_);
        format_ = nullptr;
    }

    if (audio_track_index_ < 0) {
        SetError("There is no audio stream in file with fd of %d.", fd);
        AMediaExtractor_delete(extractor_);
        return false;
    }

    AMediaFormat_getString(format_, AMEDIAFORMAT_KEY_MIME, &info);
    AMediaFormat_getInt32(format_, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channel_count);
    AMediaFormat_getInt32(format_, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sample_rate);
    AMediaFormat_getInt64(format_, AMEDIAFORMAT_KEY_DURATION, &duration_us);
    duration_ms = duration_us / 1000.0;

    LOGI("Create codec for %s. Audio Info: channel_count: %d sample_rate: %d duration_ms: %f", info,
         channel_count, sample_rate, duration_ms);
    codec_ = AMediaCodec_createDecoderByType(info);
    if (!codec_) {
        SetError("Failed to create codec for %s.", info);
        return false;
    }

    LOGI("AMediaCodec_configure...");
    media_status_t status = AMediaCodec_configure(codec_, format_, nullptr, nullptr, 0);
    if (status) {
        SetError("Error when configure mediacodec decoder, return %d", status);
        return false;
    }

    LOGI("AMediaCodec_start...");
    status = AMediaCodec_start(codec_);
    if (status) {
        SetError("Eror when start mediacodec decoder, return %d", status);
        return false;
    }

    ret = AMediaCodec_flush(codec_);
    if (ret != AMEDIA_OK) {
        SetError("Error when flush codec. return %d.", ret);
        return false;
    }
    ret = AMediaExtractor_seekTo(extractor_, 0, AMEDIAEXTRACTOR_SEEK_PREVIOUS_SYNC);
    if (ret != AMEDIA_OK) {
        SetError("Error when seek to. return %d.", ret);
        return false;
    }

    if (channel_count != m_output_channels || sample_rate != m_output_sample_rate) {
        AVChannelLayout current_channel_layout = AV_CHANNEL_LAYOUT_STEREO;
        AVChannelLayout output_channel_layout = AV_CHANNEL_LAYOUT_STEREO;

        if (channel_count == 1) current_channel_layout = AV_CHANNEL_LAYOUT_MONO;
        if (m_output_channels == 1) output_channel_layout = AV_CHANNEL_LAYOUT_MONO;

        m_converter.reset(new chromaprint::FFmpegAudioProcessor());
        m_converter->SetCompatibleMode();
        m_converter->SetInputSampleFormat(AV_SAMPLE_FMT_S16);
        m_converter->SetOutputSampleFormat(AV_SAMPLE_FMT_S16);
        m_converter->SetInputChannelLayout(&current_channel_layout);
        m_converter->SetOutputChannelLayout(&output_channel_layout);
        m_converter->SetInputSampleRate(sample_rate);
        m_converter->SetOutputSampleRate(m_output_sample_rate);
        ret = m_converter->Init();
        if (ret != 0) {
            SetError("Could not create an audio converter instance, converter ret: %d", ret);
            return false;
        }
    }

    return true;
}


bool MediaCodecReader::loopRead(
        std::function<LoopResultAction(const int16_t *data, size_t size)> callback) {
    bool extractor_eos = false;
    bool decoder_eos = false;
    int input_index = 0;
    int output_index = 0;
    int input_try_again_count = 0;
    int try_again_count = 0;

    int64_t presentation_time;
    size_t input_size = 0;
    size_t output_size = 0;
    ssize_t sample_size = 0;
    uint8_t *input = nullptr;
    uint8_t *output = nullptr;
    const int16_t *output16 = nullptr;
    AMediaCodecBufferInfo bufferInfo;
    int m_convert_buffer_nb_samples = 0;
    uint8_t *m_convert_buffer[1] = {nullptr};

    while (true) {
        if (!extractor_eos) {
            // 获取可用的inputBuffer的index
            input_index = AMediaCodec_dequeueInputBuffer(codec_, 30 * 1000);

            if (input_index >= 0) {
                input = AMediaCodec_getInputBuffer(codec_, input_index, &input_size);
                sample_size = AMediaExtractor_readSampleData(extractor_, input, input_size);

                if (sample_size < 0) {
                    sample_size = 0;
                    extractor_eos = true;
                    LOGI("Video extractor has got eof. queue EOS");
                }
                presentation_time = AMediaExtractor_getSampleTime(extractor_);
                AMediaCodec_queueInputBuffer(codec_, input_index, 0, sample_size, presentation_time,
                                             extractor_eos ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM
                                                           : 0);
                AMediaExtractor_advance(extractor_);
            } else {
                if (input_try_again_count >= 20) {
                    SetError("Dequeue input buffer return %d, try times: %d, do exit!",
                             input_index, input_try_again_count++);
                    return false;
                }
                LOGI("Dequeue input buffer return %d, try it again later. try times: %d",
                     input_index, input_try_again_count++);
            }
        } else {
            LOGI("Video extractor has got eof. do nothing.");
        }

        if (!decoder_eos) {
            output_index = AMediaCodec_dequeueOutputBuffer(codec_, &bufferInfo, 30 * 1000);
            if (output_index >= 0) {
                try_again_count = 0;
//                LOGI("Output succeed.");
                if (bufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                    decoder_eos = true;
                    LOGI("Decoder has got eof.");
                }

                if (!decoder_eos) {
                    output = AMediaCodec_getOutputBuffer(codec_, output_index, nullptr);

                    // uint8_t到int16_t的转换使buffer总数减半，故除二；
                    // 需要计算per_channel，故除channel_count;
                    int nb_samples_per_channel = bufferInfo.size / 2 / channel_count;

                    if (m_converter) {
                        int ret = 0;
                        if (nb_samples_per_channel > m_convert_buffer_nb_samples) {
                            int linsize;
                            av_freep(&m_convert_buffer[0]);
                            m_convert_buffer_nb_samples = std::max(1024 * 8,
                                                                   nb_samples_per_channel);
                            ret = av_samples_alloc(m_convert_buffer, &linsize, channel_count,
                                                   m_convert_buffer_nb_samples,
                                                   AV_SAMPLE_FMT_S16,
                                                   1);
                            if (ret < 0) {
                                SetError("Couldn't allocate audio converter buffer", ret);
                                return false;
                            }
                        }
                        auto nb_samples = m_converter->Convert(m_convert_buffer,
                                                               m_convert_buffer_nb_samples,
                                                               (const uint8_t **) &output,
                                                               nb_samples_per_channel);
                        if (nb_samples < 0) {
                            SetError("Couldn't convert audio, ret: %d", ret);
                            return false;
                        }

                        output16 = (const int16_t *) m_convert_buffer[0];
                        output_size = nb_samples;
                    } else {
                        output16 = (const int16_t *) output;
                        output_size = nb_samples_per_channel;
                    }

                    // 回调外部，外部使用完buffer数据后再释放buffer，根据回调的返回值决定下一步
                    LoopResultAction resultAction = callback(output16, output_size);
                    if (resultAction == LoopResultAction::STOPPED) {
                        return false;
                    } else if (resultAction == LoopResultAction::FINISHED) {
                        return true;
                    }

                    AMediaCodec_releaseOutputBuffer(codec_, output_index, false);
                }
            } else if (output_index == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
                LOGI("Output buffers changed.");
            } else if (output_index == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                LOGI("Output format changed.");
                try_again_count = 0;
            } else if (output_index == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                LOGI("Try again later. try_again_count = %d.", try_again_count);
                try_again_count++;
                if (try_again_count > 40) {
                    SetError("Try again 20 times continously. consider it failed.");
                    return false;
                }
            } else {
                LOGI("Unexpected info code %d", output_index);
            }

            continue;
        }

        break;
    }

    return true;
}

void MediaCodecReader::Close() {
    if (codec_ != nullptr) {
        AMediaCodec_stop(codec_);
        AMediaCodec_delete(codec_);
        codec_ = nullptr;
    }
    if (extractor_ != nullptr) {
        AMediaExtractor_delete(extractor_);
        extractor_ = nullptr;
    }
    if (format_ != nullptr) {
        AMediaFormat_delete(format_);
        format_ = nullptr;
    }
}
