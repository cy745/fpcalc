//
// Created by Qiu on 2024/3/25.
//
#ifndef FPCALC_FPCALC_JNI_H
#define FPCALC_FPCALC_JNI_H

#include <jni.h>
#include <stdlib.h>
#include <android/log.h>

#define LOGI(...)   __android_log_print((int)ANDROID_LOG_INFO, "FPCALC", __VA_ARGS__)
#define LOGE(...)   __android_log_print((int)ANDROID_LOG_ERROR, "FPCALC", __VA_ARGS__)

struct FpcalcParams {
    int target_fd = -1;
    int g_max_duration = 120;
    int g_algorithm = 2;
    bool g_raw = false;
    bool g_signed = false;
};

struct FpcalcResult {
    const char *fingerprint = nullptr;
    const char *raw_fingerprint = nullptr;
    char *error_message = new char[1024];
    long source_duration_ms = 0;
    int source_sample_rate = 0;
    int source_channels = 0;
    int source_length = 0;
};

inline FpcalcParams *jobjectToStruct(JNIEnv *env, jobject thiz, jobject params) {
    jclass paramsClass = env->FindClass("com/lalilu/fpcalc/FpcalcParams");
    jfieldID target_fd_id = env->GetFieldID(paramsClass, "targetFd", "I");
    jfieldID g_max_duration_id = env->GetFieldID(paramsClass, "gMaxDuration", "I");
    jfieldID g_raw_id = env->GetFieldID(paramsClass, "gRaw", "Z");
    jfieldID g_signed_id = env->GetFieldID(paramsClass, "gSigned", "Z");
    jfieldID g_algorithm = env->GetFieldID(paramsClass, "gAlgorithm", "I");

    FpcalcParams *params_ = new FpcalcParams();
    params_->target_fd = env->GetIntField(params, target_fd_id);
    params_->g_max_duration = env->GetIntField(params, g_max_duration_id);
    params_->g_raw = env->GetBooleanField(params, g_raw_id);
    params_->g_signed = env->GetBooleanField(params, g_signed_id);
    params_->g_algorithm = env->GetIntField(params, g_algorithm);
    return params_;
}

inline jobject structToJobject(JNIEnv *env, jobject thiz, FpcalcResult *result) {
    jclass resultClass = env->FindClass("com/lalilu/fpcalc/FpcalcResult");
    jfieldID fingerprint_id = env->GetFieldID(resultClass, "fingerprint", "Ljava/lang/String;");
    jfieldID raw_fingerprint_id = env->GetFieldID(resultClass, "rawFingerprint",
                                                  "Ljava/lang/String;");
    jfieldID error_message_id = env->GetFieldID(resultClass, "errorMessage", "Ljava/lang/String;");
    jfieldID source_duration_ms_id = env->GetFieldID(resultClass, "sourceDurationMs", "J");
    jfieldID source_sample_rate_id = env->GetFieldID(resultClass, "sourceSampleRate", "I");
    jfieldID source_channels_id = env->GetFieldID(resultClass, "sourceChannels", "I");
    jfieldID source_length_id = env->GetFieldID(resultClass, "sourceLength", "I");

    jstring error_message = env->NewStringUTF(result->error_message);
    free(result->error_message);

    jobject resultObj = env->AllocObject(resultClass);
    if (result->fingerprint != nullptr) {
        jstring fingerprint = env->NewStringUTF(result->fingerprint);
        env->SetObjectField(resultObj, fingerprint_id, fingerprint);
    }
    if (result->raw_fingerprint != nullptr) {
        jstring raw_fingerprint = env->NewStringUTF(result->raw_fingerprint);
        env->SetObjectField(resultObj, raw_fingerprint_id, raw_fingerprint);
    }
    env->SetObjectField(resultObj, error_message_id, error_message);
    env->SetLongField(resultObj, source_duration_ms_id, (jlong) result->source_duration_ms);
    env->SetIntField(resultObj, source_sample_rate_id, (jint) result->source_sample_rate);
    env->SetIntField(resultObj, source_channels_id, (jint) result->source_channels);
    env->SetIntField(resultObj, source_length_id, (jint) result->source_length);

    free(result);
    return resultObj;
}


#endif //FPCALC_FPCALC_JNI_H
