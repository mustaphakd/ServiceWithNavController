package com.wrsft.servicewithnavcontroller

@Suppress("KotlinJniMissingFunction")
class NativeWrapper {

    external fun startapp(directory: String) //: Void
    external fun stopapp() //: Void

    fun startWrapperApp() {
        startapp("none")
    }

    fun stopWrapperApp() {
        stopapp()
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}