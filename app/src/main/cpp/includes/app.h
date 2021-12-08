//
// Created by musta on 11/13/2021.
//

#ifndef SERVICEWITHNAVCONTROLLER_APP_H
#define SERVICEWITHNAVCONTROLLER_APP_H


#include <time.h>
//#include <stdlib.h>
/**
 * includes following below:
 * #include <sys/cdefs.h>

/* wchar_t is required in stdlib.h according to POSIX.
 * note that defining __need_wchar_t prevents stddef.h
 * to define all other symbols it does normally *
#define __need_wchar_t
#include <stddef.h>

#include <stddef.h>
#include <string.h>
#include <alloca.h>
#include <strings.h>
#include <memory.h>
 */

#include <__config>

//#include <inttypes.h>
#include <jni.h>
#include <android/log.h>
#include <string>
#include "webServer.h"
#include <memory>
//#include <assert.h>

#include <iostream>
#include <vector>
//#include <filesystem>

#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time

#include <fs/filesystem.hpp>
//namespace fs = std::filesystem;

namespace fs = ghc::filesystem;

#define  LOG_TAG    "wrsft-ServiceWIthNavController" // or static const char* kTAG = "hello-jniCallback";
#define  LOGI(...)  ((void)__android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__))
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/* Set to 1 to enable debug log traces or 0 otherwise. */
#define DEBUG 1

extern "C" {
    void start_app( JNIEnv *pEnv, jobject pObj, jstring directoryName);// __unused jstring directoryName
    void stop_app(JNIEnv *pEnv, jobject pObj);
}

namespace wrsft {

    class Application {
        public:
        const int FILE_SIZE = 2 * 1024 * 1024;
        static Application& get_instance(const std::string path);
        static void cleanUp();


        //Settings > System > Developer options > Logger buffer sizes and choose a higher value.
        static void write_log(const std::string methodName, const std::string message);

        void start(); // start thread
        void stop(); // stop thread

        // see what happens to instance before implementing cleanup method
        ~Application(); /*{
            wrsft::Application::write_log("Application::Destructor ~Application", "start- stop");
        }*/

        private:
        const std::string directory_path;
        static Application * instance ;

        fs::fstream * logFile ;

        WebServer<5> webServer;

        Application() = delete;
        Application(const std::string);
        Application(const Application& other) = delete;
        Application& operator=(const Application& other) = delete;



        std::vector<std::string> getFiles(std::string path);
        int getStreamSize(std::string path);
        void writeToFile(std::string content);

        void openOrCreateFile(std::string );
    };

}

// LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
#endif //SERVICEWITHNAVCONTROLLER_APP_H
