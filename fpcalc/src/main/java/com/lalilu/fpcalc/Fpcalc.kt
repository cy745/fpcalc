package com.lalilu.fpcalc

object Fpcalc {
    // Used to load the 'fpcalc' library on application startup.
    init {
        System.loadLibrary("fpcalc")
    }

    external fun calc(params: FpcalcParams): FpcalcResult
}

data class FpcalcParams(
    val targetFd: Int = -1,
    val targetFilePath: String? = null,
    val gMaxDuration: Int = 120,    // in second
    val gRaw: Boolean = false,
    val gSigned: Boolean = false,
    val gAlgorithm: Int = 2,
)

class FpcalcResult {
    var fingerprint: String? = null
    var rawFingerprint: String? = null
    var errorMessage: String? = null
    var sourceDurationMs: Long = 0
    var sourceSampleRate: Int = 0
    var sourceChannels: Int = 0
    var sourceLength: Int = 0
    override fun toString(): String {
        return "FpcalcResult(fingerprint=$fingerprint, rawFingerprint=$rawFingerprint, errorMessage=$errorMessage, sourceDurationMs=$sourceDurationMs, sourceSampleRate=$sourceSampleRate, sourceChannels=$sourceChannels, sourceLength=$sourceLength)"
    }
}