package com.lalilu.fpcalc

object Fpcalc {
    // Used to load the 'fpcalc' library on application startup.
    init {
        System.loadLibrary("fpcalc")
    }

    /**
     * A native method that is implemented by the 'fpcalc' native library,
     * which is packaged with this application.
     */
    external fun calc(args: Array<String>): String
}