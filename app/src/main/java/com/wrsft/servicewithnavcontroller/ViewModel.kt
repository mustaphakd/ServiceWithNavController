package com.wrsft.servicewithnavcontroller

import android.app.ActivityManager
import android.content.Context
import android.content.Intent
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import java.io.File
import java.io.FileOutputStream
import java.lang.ref.WeakReference
import java.util.logging.Logger

class ViewModel () : ViewModel() {
    private val _canStart = MutableStateFlow<Boolean>(false)
    private val _canStop = MutableStateFlow<Boolean>(false)
    val canStart : StateFlow<Boolean> = _canStart
    val canStop: StateFlow<Boolean> = _canStop

    var contextRef: WeakReference<android.content.Context>? = null

    private var log: Logger

    init {

        //canStart.set(false)
        //canStop.set(false)

        log = Logger.getLogger(ViewModel::class.java.name)
        log.info("init() ViewModel:")

    }

    public fun runScope() {
        var logCounter = 0

        viewModelScope.launch {
            serviceDetectionFlow.collect { newVal ->
                if(logCounter++ >= 100)
                    log.info("serviceDetectionFlow.collect newVal: ${newVal}")
                _canStart.value = (!newVal)
                _canStop.value = newVal
                if(logCounter >= 100) {
                    log.info("serviceDetectionFlow.collect canStart: ${canStart.value}")
                    log.info("serviceDetectionFlow.collect canStop: ${canStop.value}")
                    logCounter = -1000
                }
            }
        }
    }

    private fun loadResources() {
        log.info("loading resources.... Start")

        val context : Context? = contextRef?.get()

        if (context == null)
            throw Error("Failed to acquired Activity context.")

        val ext: File? =context.getExternalFilesDir(null)
        val tempDir = File(ext, "res/")
        tempDir.mkdirs()

        if (!tempDir.exists() || !tempDir.canWrite()) {
            throw Error("Failed to open res directory");
        }

        copyResourceToDestination(tempDir, "index.html", R.raw.index, context)
        copyResourceToDestination(tempDir, "cert.pem", R.raw.cert, context)
        copyResourceToDestination(tempDir, "key.pem", R.raw.key, context)
        copyResourceToDestination(tempDir, "index.brot", R.raw.indexbrot, context)


        log.info("loading resources.... Start")
    }

    fun copyResourceToDestination(destDir: File, destFileName: String, sourceFd: Int, context: Context){
        val dstFullpath =  File(destDir, destFileName)
        if(! dstFullpath.exists())
        {
            context.resources.openRawResource(sourceFd)
            .copyTo(FileOutputStream(dstFullpath))
        }
    }

    private val serviceDetectionFlow : Flow<Boolean> = flow {
       // var firstRun = true
        var logCounter = 0;

        while (true) {
            var context = contextRef?.get()

            if (context != null) {
                val isServiceRunning = isServiceRunning()
                //var previousValue = canStop.value

                if(logCounter++ >= 10) {
                    log.info("viewmodel i service running? : ${isServiceRunning}")
                    logCounter = -10
                }

               //if (previousValue != isServiceRunning || firstRun == true) {
                   //firstRun = false
                   emit(isServiceRunning)
                  // previousValue = !previousValue
               //}
            }

            delay(13000)
        }
    }

    fun setContext(context: android.content.Context){
        contextRef = WeakReference<android.content.Context>(context)

        loadResources()
    }


    @Suppress("DEPRECATION")
    private  suspend fun isServiceRunning() : Boolean  =
        (contextRef?.get()?.getSystemService(AppCompatActivity.ACTIVITY_SERVICE) as ActivityManager)
            .getRunningServices(Integer.MAX_VALUE)
            .any { it.service.className == ForegroundService::class.java.name }



    fun startService() {

        log.info("startService canStart is ${canStart}")
        if((canStart.value as Boolean)== false)
            return

        var context = contextRef?.get()
        val intent = Intent(context, ForegroundService::class.java )

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context?.startForegroundService(intent)
        }
        else
        {
            context?.startService(intent)
        }

       // _canStart.value = false
        //_canStop.value = true
    }

    fun stopService() {

        log.info("stopService canStop is ${canStop}")
        if(canStop.value== false) return


        var context = contextRef?.get()
        val intent = Intent(context, ForegroundService::class.java )

        context?.stopService(intent)

        //_canStart.value = true
        //_canStop.value = false
    }

    fun open() {

    }


    private fun getTempFiles(): Array<File>? {

        var context = contextRef?.get()
        val ext: File? = context?.getExternalFilesDir(null)
        val tempDir = File(ext, "logs/")
        tempDir.mkdirs()

        if (!tempDir.exists() || !tempDir.canWrite()) {
            return null
        }

        val files = tempDir.listFiles()

        files?.forEach {
            println("existing file found in logs: ${it.name}    =>  ${it.length()} bytes")
        }
        return files
    }

}