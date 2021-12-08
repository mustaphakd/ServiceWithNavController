package com.wrsft.servicewithnavcontroller

@Suppress("KotlinJniMissingFunction")
class NativeWrapper {

    external fun startapp(directory: String) //: Void
    external fun stopapp() //: Void

    fun startWrapperApp(tempdir: String) {
        startapp(tempdir)
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