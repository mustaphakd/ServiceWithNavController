package com.wrsft.servicewithnavcontroller

import android.app.*
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.core.app.NotificationCompat
import java.io.File
import java.text.SimpleDateFormat
import java.util.*
import java.util.logging.Logger

class ForegroundService : Service() {

    private var exitThreads: Boolean = false
    private  lateinit var log: Logger
    private lateinit var currentLocalTime: Date
    private val dateTimeFormat = SimpleDateFormat("yyyyMMdd_HHmmss_z", Locale.ENGLISH)
    private val cal: Calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT+0:00"))

    override fun onCreate() {
        super.onCreate()

        log = Logger.getLogger(ForegroundService::class.java.name)
        showToast("App starting with on Create called")
        log.info("onCreate() bundleData:")
    }

    override fun onBind(p0: Intent?): IBinder? {
        showToast("onBind")
        log.info("onBind() bundleData:")
        return null
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        showToast("onStartCommand")

        val pendingIntent: PendingIntent =
            Intent(this, MainActivity::class.java).let { notificationIntent -> //ForegroundService
                PendingIntent.getActivity(this, 0, notificationIntent, if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O){ PendingIntent.FLAG_MUTABLE}else{ 0} ) //PendingIntent.FLAG_MUTABLE
            }

        val notification: Notification = getNotificationBuilder()
            .setContentTitle(getText(R.string.notification_title))
            .setContentText(getText(R.string.notification_message))
            .setSmallIcon(R.drawable.ic_launcher_background)
            .setContentIntent(pendingIntent)
            .setTicker(getText(R.string.ticker_text))
            .build()

        startThreads()
// Notification ID cannot be 0.
        startForeground(4445672, notification)

       // return super.onStartCommand(intent, flags, startId)
        return  START_STICKY
    }


    private lateinit var writingThread: Thread

    private  fun startThreads() {

        currentLocalTime = cal.time

        writingThread = Thread {
            writeToFile()
        }

        val temDir = getLogDirectory()
        val wrapper = NativeWrapper()
        wrapper.startWrapperApp(temDir.absolutePath)

        writingThread.start()
    }

    private  fun writeToFile() {
        val buffer = StringBuilder()
        while (true)
        {
            Thread.sleep(50000)

           /* getCurrentDateTime(buffer)
            buffer.append("-- WriteFile running .... \r\n")  //currentLog.appendText(buffer.toString())

            log.info("service running.. ${buffer.toString()}")
            buffer.clear() */

            if(exitThreads) break
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        exitThreads = true


        val wrapper = NativeWrapper()
        wrapper.stopWrapperApp()
    }

    override fun stopService(name: Intent?): Boolean {
        log.info("stop service requested")

        exitThreads = true
        return super.stopService(name)
    }

    private fun getLogDirectory(): File {
        val ext: File? = getExternalFilesDir(null)
        val tempDir = File(ext, "logs/")
        tempDir.mkdirs()

        if (!tempDir.exists() || !tempDir.canWrite()) {
            throw Error("Failed to open logs directory");
        }

        return ext!!
    }

    @RequiresApi(Build.VERSION_CODES.O)
    private fun createNotificationChannel(): String{
        val channelId = "my_service"
        val channelName = "My Background Service"
        val chan = NotificationChannel(channelId,
            channelName, NotificationManager.IMPORTANCE_HIGH)
        //chan.lightColor = Color.BLUE
        chan.importance = NotificationManager.IMPORTANCE_NONE
        chan.lockscreenVisibility = Notification.VISIBILITY_PRIVATE
        val service = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        service.createNotificationChannel(chan)
        return channelId
    }

    private fun getNotificationBuilder()  : NotificationCompat.Builder  {    //Notification.Builder{
        /*return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, "CHANNEL_DEFAULT_IMPORTANCE")
        } else {
            Notification.Builder(this)
        }*/

        val channelId =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                createNotificationChannel()
            } else {
                // If earlier version channel ID is not used
                // https://developer.android.com/reference/android/support/v4/app/NotificationCompat.Builder.html#NotificationCompat.Builder(android.content.Context)
                ""
            }

        return NotificationCompat.Builder(this, channelId)
    }

    private fun getCurrentDateTime(buffer: StringBuilder) {
        // try {            //dateTimeFormat.timeZone = TimeZone.getTimeZone("GMT+1:00")
        currentLocalTime = cal.time
        buffer.append(dateTimeFormat.format(currentLocalTime))
        //cal.clear()
        //return localTime
        // }finally {

        //}
    }

    private fun showToast(content: String) {
        Toast.makeText(this, content, Toast.LENGTH_LONG).show()

    }

}