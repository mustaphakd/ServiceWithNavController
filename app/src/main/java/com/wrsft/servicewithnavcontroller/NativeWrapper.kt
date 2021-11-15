package com.wrsft.servicewithnavcontroller

@Suppress("KotlinJniMissingFunction")
class NativeWrapper {

    external fun start_app(directoryPath: String): Void
    external fun stop_app(): Void

    fun startApp() {
        start_app("none")
    }

    fun stopApp() {
        stop_app()
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}