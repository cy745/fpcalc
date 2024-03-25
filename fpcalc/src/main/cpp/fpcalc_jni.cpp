#include "fpcalc_jni.h"

extern FpcalcResult *fpcalc_main(FpcalcParams *params);

extern "C"
JNIEXPORT jobject JNICALL
Java_com_lalilu_fpcalc_Fpcalc_calc(JNIEnv *env, jobject thiz, jobject params) {
    FpcalcParams *params_ = jobjectToStruct(env, thiz, params);
    FpcalcResult *result = fpcalc_main(params_);
    return structToJobject(env, thiz, result);
}
