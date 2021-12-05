package com.wrsft.servicewithnavcontroller

import android.app.ActivityManager
import android.content.Intent
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.BaseObservable
import androidx.databinding.ObservableField
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.google.android.play.core.tasks.Task
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Delay
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
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

        viewModelScope.launch {
            serviceDetectionFlow.collect { newVal ->
                log.info("serviceDetectionFlow.collect newVal: ${newVal}")
                _canStart.value = (!newVal)
                _canStop.value = newVal
                log.info("serviceDetectionFlow.collect canStart: ${canStart.value}")
                log.info("serviceDetectionFlow.collect canStop: ${canStop.value}")
            }
        }
    }

    private val serviceDetectionFlow : Flow<Boolean> = flow {
        var firstRun = true

        while (true) {
            var context = contextRef?.get()

            if (context != null) {
                val isServiceRunning = isServiceRunning()
                var previousValue = canStop.value

                log.info("viewmodel i service running? : ${isServiceRunning}")

               //if (previousValue != isServiceRunning || firstRun == true) {
                   firstRun = false
                   emit(isServiceRunning)
                   previousValue = !previousValue
               //}
            }

            delay(13000)
        }
    }

    fun setContext(context: android.content.Context){
        contextRef = WeakReference<android.content.Context>(context)
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

}