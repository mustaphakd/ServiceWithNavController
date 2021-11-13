package com.wrsft.servicewithnavcontroller

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.Build
import java.util.logging.Logger

class AutoStarter : BroadcastReceiver() {
    private  var log: Logger

    init {

        log = Logger.getLogger(AutoStarter::class.java.name);
        log.info("Init() bundleData:")
    }
    override fun onReceive(p0: Context?, p1: Intent?) {

        log.info("onReceive ..... Build version : ${Build.VERSION.SDK_INT}")
        val intent = Intent(p0, ForegroundService::class.java)

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            p0?.startForegroundService(intent)
        }
        else
            p0?.startService(intent)
    }
}