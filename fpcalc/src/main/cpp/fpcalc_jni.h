//
// Created by Qiu on 2024/3/25.
//
#ifndef FPCALC_FPCALC_JNI_H
#define FPCALC_FPCALC_JNI_H

#include <jni.h>
#include <cstdlib>
#include <android/log.h>
#include <cstring>

#define LOGI(...)   __android_log_print((int)ANDROID_LOG_INFO, "FPCALC", __VA_ARGS__)
#define LOGE(...)   __android_log_print((int)ANDROID_LOG_ERROR, "FPCALC", __VA_ARGS__)

struct FpcalcParams {
    int target_fd = -1;
    const char *target_file_path = nullptr;
    int g_max_duration = 120;
    int g_algorithm = 2;
    bool g_raw = false;
    bool g_signed = false;
};

struct FpcalcResult {
    char *fingerprint = nullptr;
    char *raw_fingerprint = nullptr;
    char *error_message = nullptr;
    long source_duration_ms = 0;
    int source_sample_rate = 0;
    int source_channels = 0;
    int source_length = 0;

    ~FpcalcResult() {
        free((void *) fingerprint);
        free((void *) raw_fingerprint);
        free((void *) error_message);
    }

    bool printFingerprint(char *str, bool is_raw) {
        size_t char_size = strlen(str);
        if (is_raw) {
            raw_fingerprint = (char *) malloc(char_size * sizeof(char));
            return strcpy(raw_fingerprint, str) != nullptr;
        } else {
            fingerprint = (char *) malloc(char_size * sizeof(char));
            return strcpy(fingerprint, str) != nullptr;
        }
    }

    bool printError(const char *format, ...) {
        va_list args;
        va_start(args, format);

        int max_length = 100;
        error_message = (char *) malloc(max_length * sizeof(char)); // 分配内存
        if (error_message == nullptr) {
            LOGE("Memory allocation for error_message failed.");
            va_end(args);
            return false;
        }
        int result = snprintf(error_message, max_length, format, args);
        if (result > max_length) {
            error_message = (char *) realloc(
                    error_message, (result + 1) * sizeof(char)); // 重新分配内存以容纳完整的字符串

            if (error_message == nullptr) {
                LOGE("Memory reallocation for error_message failed");
                va_end(args);
                return false;
            }

            result = snprintf(error_message, result + 1, format, args); // 再次尝试打印字符串
        }
        va_end(args);
        return true;
    }
};

inline FpcalcParams *jobjectToStruct(JNIEnv *env, jobject thiz, jobject params) {
    jboolean is_copy = JNI_TRUE;

    jclass paramsClass = env->FindClass("com/lalilu/fpcalc/FpcalcParams");
    jfieldID target_fd_id = env->GetFieldID(paramsClass, "targetFd", "I");
    jfieldID target_file_path_id = env->GetFieldID(paramsClass, "targetFilePath",
                                                   "Ljava/lang/String;");
    jfieldID g_max_duration_id = env->GetFieldID(paramsClass, "gMaxDuration", "I");
    jfieldID g_raw_id = env->GetFieldID(paramsClass, "gRaw", "Z");
    jfieldID g_signed_id = env->GetFieldID(paramsClass, "gSigned", "Z");
    jfieldID g_algorithm = env->GetFieldID(paramsClass, "gAlgorithm", "I");

    auto *params_ = new FpcalcParams();
    params_->target_fd = env->GetIntField(params, target_fd_id);

    auto target_file_path_str = (jstring) env->GetObjectField(params, target_file_path_id);
    if (target_file_path_str != nullptr) {
        params_->target_file_path = env->GetStringUTFChars(target_file_path_str, &is_copy);
    }

    params_->g_max_duration = env->GetIntField(params, g_max_duration_id);
    params_->g_raw = env->GetBooleanField(params, g_raw_id);
    params_->g_signed = env->GetBooleanField(params, g_signed_id);
    params_->g_algorithm = env->GetIntField(params, g_algorithm);
    return params_;
}

inline jobject structToJobject(JNIEnv *env, jobject thiz, FpcalcResult *result) {
    jclass resultClass = env->FindClass("com/lalilu/fpcalc/FpcalcResult");
    jobject resultObj = env->AllocObject(resultClass);

    if (result->error_message != nullptr) {
        jfieldID error_message_id = env->GetFieldID(resultClass, "errorMessage",
                                                    "Ljava/lang/String;");
        jstring error_message = env->NewStringUTF(result->error_message);
        env->SetObjectField(resultObj, error_message_id, error_message);
    }

    if (result->fingerprint != nullptr) {
        jfieldID fingerprint_id = env->GetFieldID(resultClass, "fingerprint", "Ljava/lang/String;");
        jstring fingerprint = env->NewStringUTF(result->fingerprint);
        env->SetObjectField(resultObj, fingerprint_id, fingerprint);
    }

    if (result->raw_fingerprint != nullptr) {
        jfieldID raw_fingerprint_id = env->GetFieldID(resultClass, "rawFingerprint",
                                                      "Ljava/lang/String;");
        jstring raw_fingerprint = env->NewStringUTF(result->raw_fingerprint);
        env->SetObjectField(resultObj, raw_fingerprint_id, raw_fingerprint);
    }

    jfieldID source_duration_ms_id = env->GetFieldID(resultClass, "sourceDurationMs", "J");
    jfieldID source_sample_rate_id = env->GetFieldID(resultClass, "sourceSampleRate", "I");
    jfieldID source_channels_id = env->GetFieldID(resultClass, "sourceChannels", "I");
    jfieldID source_length_id = env->GetFieldID(resultClass, "sourceLength", "I");
    env->SetLongField(resultObj, source_duration_ms_id, (jlong) result->source_duration_ms);
    env->SetIntField(resultObj, source_sample_rate_id, (jint) result->source_sample_rate);
    env->SetIntField(resultObj, source_channels_id, (jint) result->source_channels);
    env->SetIntField(resultObj, source_length_id, (jint) result->source_length);

    free(result);
    return resultObj;
}


#endif //FPCALC_FPCALC_JNI_H
