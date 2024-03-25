//
// Created by Qiu on 2024/3/24.
//
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <chrono>
#include <chromaprint.h>
#include "utils/scope_exit.h"
#include "fpcalc_reader.h"
#include "fpcalc_jni.h"

using namespace chromaprint;

static double g_max_duration = 120;
static bool g_raw = false;
static bool g_signed = false;
static ChromaprintAlgorithm g_algorithm = CHROMAPRINT_ALGORITHM_DEFAULT;

const char *g_help =
        "Usage: %s [OPTIONS] FILE [FILE...]\n"
        "\n"
        "Generate fingerprints from audio files/streams.\n"
        "\n"
        "Options:\n"
        "  -length SECS   Restrict the duration of the processed input audio (default 120)\n"
        "  -algorithm NUM Set the algorithm method (default 2)\n"
        "  -raw           Output fingerprints in the uncompressed format\n"
        "  -signed        Change the uncompressed format from unsigned integers to signed (for pg_acoustid compatibility)\n";

void PrintResult(ChromaprintContext *ctx, MediaCodecReader &reader, FpcalcResult *result) {
    std::string tmp_fp;
    const char *fp;
    bool dealloc_fp = false;

    int size;
    if (!chromaprint_get_raw_fingerprint_size(ctx, &size)) {
        sprintf(result->error_message, "ERROR: Could not get the fingerprinting size\n");
        return;
    }
    if (size <= 0) {
        sprintf(result->error_message, "ERROR: Empty fingerprint\n");
        return;
    }

    if (g_raw) {
        std::stringstream ss;
        uint32_t *raw_fp_data = nullptr;
        int raw_fp_size = 0;
        if (!chromaprint_get_raw_fingerprint(ctx, &raw_fp_data, &raw_fp_size)) {
            sprintf(result->error_message, "ERROR: Could not get the fingerprinting\n");
            return;
        }
        SCOPE_EXIT(chromaprint_dealloc(raw_fp_data));
        for (int i = 0; i < raw_fp_size; i++) {
            if (i > 0) {
                ss << ',';
            }
            if (g_signed) {
                ss << static_cast<int32_t>(raw_fp_data[i]);
            } else {
                ss << raw_fp_data[i];
            }
        }
        tmp_fp = ss.str();
        fp = new char[tmp_fp.size() + 1];
        strcat((char *) fp, tmp_fp.c_str());
        dealloc_fp = true;
    } else {
        char *tmp_fp2;
        if (!chromaprint_get_fingerprint(ctx, &tmp_fp2)) {
            sprintf(result->error_message, "ERROR: Could not get the fingerprinting\n");
            return;
        }
        fp = tmp_fp2;
        dealloc_fp = true;
    }
    SCOPE_EXIT(if (dealloc_fp) { chromaprint_dealloc((void *) fp); });

    if (g_raw) {
        result->raw_fingerprint = fp;
    } else {
        result->fingerprint = fp;
    }
}

void ProcessFile(ChromaprintContext *ctx, MediaCodecReader &reader, FpcalcResult *result, int fd) {
    if (!reader.Open(fd)) {
        sprintf(result->error_message, "ERROR: %s\n", reader.GetError().c_str());
        return;
    }

    result->source_sample_rate = reader.GetSampleRate();
    result->source_channels = reader.GetChannels();
    result->source_duration_ms = reader.GetDuration();

    if (!chromaprint_start(ctx, reader.GetOutputSampleRate(), reader.GetOutputChannels())) {
        sprintf(result->error_message, "ERROR: Could not initialize the fingerprinting process\n");
        return;
    }

    size_t chunk_size = 0;
    size_t stream_size = 0;
    const size_t stream_limit = g_max_duration * reader.GetOutputSampleRate();

    bool decode_result = reader.loopRead([&](const int16_t *frame_data, size_t frame_size) {
        bool stream_done = false;
        if (stream_limit > 0) {
            const auto remaining = stream_limit - stream_size;
            if (frame_size > remaining) {
                frame_size = remaining;
                stream_done = true;
            }
        }
        stream_size += frame_size;

        if (frame_size == 0) {
            if (stream_done) {
                return LoopResultAction::FINISHED;
            } else {
                return LoopResultAction::CONTINUE;
            }
        }

        size_t first_part_size = frame_size;

        if (!chromaprint_feed(ctx, frame_data, first_part_size * reader.GetOutputChannels())) {
            sprintf(result->error_message, "ERROR: Could not process audio data\n");
            return LoopResultAction::STOPPED;
        }

        chunk_size += first_part_size;
        frame_data += first_part_size * reader.GetOutputChannels();
        frame_size -= first_part_size;

        if (frame_size > 0) {
            if (!chromaprint_feed(ctx, frame_data, frame_size * reader.GetOutputChannels())) {
                sprintf(result->error_message, "ERROR: Could not process audio data\n");
                return LoopResultAction::STOPPED;
            }
        }

        chunk_size += frame_size;

        if (stream_done) {
            return LoopResultAction::FINISHED;
        }

        return LoopResultAction::CONTINUE;
    });

    if (!decode_result) {
        sprintf(result->error_message, "ERROR: %s\n", reader.GetError().c_str());
        return;
    }

    if (!chromaprint_finish(ctx)) {
        sprintf(result->error_message, "ERROR: Could not finish the fingerprinting process\n");
        return;
    }

    if (chunk_size > 0) {
        PrintResult(ctx, reader, result);
    } else {
        sprintf(result->error_message, "ERROR: Not enough audio data\n");
    }
}

static bool ProcessParams(FpcalcParams *params) {
    g_algorithm = (ChromaprintAlgorithm) (params->g_algorithm - 1);
    g_raw = params->g_raw;
    g_max_duration = params->g_max_duration;
    g_signed = params->g_signed;
    return params->target_fd > 0;
}

FpcalcResult *fpcalc_main(FpcalcParams *params) {
    FpcalcResult *result = new FpcalcResult();

    if (!ProcessParams(params)) {
        result->error_message = (char *) "File descriptor is not specified";
        return result;
    }

    ChromaprintContext *chromaprint_ctx = chromaprint_new(g_algorithm);
    SCOPE_EXIT(chromaprint_free(chromaprint_ctx));

    MediaCodecReader reader;
    reader.SetOutputChannels(chromaprint_get_num_channels(chromaprint_ctx));
    reader.SetOutputSampleRate(chromaprint_get_sample_rate(chromaprint_ctx));
    ProcessFile(chromaprint_ctx, reader, result, params->target_fd);

    return result;
}
