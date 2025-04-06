package com.lalilu.fpcalc

object Fpcalc {
    // Used to load the 'fpcalc' library on application startup.
    init {
        System.loadLibrary("fpcalc")
    }

    external fun calc(params: FpcalcParams): FpcalcResult
}

/**
 * Parameters for fpcalc
 *
 * @param targetFd          File descriptor of the file to calculate fingerprint
 * @param targetFilePath    Path of the file to calculate fingerprint
 * @param gMaxDuration      Max duration of the file to calculate fingerprint
 * @param gRaw              Whether to output raw fingerprint
 * @param gSigned           Whether to output signed fingerprint
 * @param gAlgorithm        Algorithm to use for fingerprint calculation
 */
data class FpcalcParams(
    val targetFd: Int = -1,
    val targetFilePath: String? = null,
    val gMaxDuration: Int = 120,    // in second
    val gRaw: Boolean = false,
    val gSigned: Boolean = false,
    val gAlgorithm: Int = 2,
)

class FpcalcResult {
    val fingerprint: String? = null
    val rawFingerprint: String? = null
    val errorMessage: String? = null
    val sourceDurationMs: Long = 0
    val sourceSampleRate: Int = 0
    val sourceChannels: Int = 0
    val sourceLength: Int = 0

    override fun toString(): String {
        return "FpcalcResult(fingerprint=$fingerprint, rawFingerprint=$rawFingerprint, errorMessage=$errorMessage, sourceDurationMs=$sourceDurationMs, sourceSampleRate=$sourceSampleRate, sourceChannels=$sourceChannels, sourceLength=$sourceLength)"
    }
}