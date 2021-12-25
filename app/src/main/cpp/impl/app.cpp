//#include <__config>
//
// Created by musta on 11/13/2021.
//
#include "app.h"
#include "jni.h"
#include "android/looper.h"

#include <pthread.h>

namespace fs = ghc::filesystem;


namespace wrsft {

    Application * Application::instance = nullptr;
    JniContext Application::g_ctx = {0};

    Application::Application(const std::string str):
        directory_path{str}, writeCounter{0}, current_file{""}, webServer{ WebServer<5>(&Application::write_log, getResDirPath(directory_path))}, logFile{nullptr}
    {

        fs::path rootdir =  { directory_path};
        fs::path logdir = rootdir / "logs";

        log_directory_path = logdir.string();

        Application::write_log("Application::ctr", "directory path and webserver instantiated :== " + directory_path);
        Application::write_log("Application::ctr", "log directory path and webserver instantiated :== " + log_directory_path);
        Application::write_log("Application::ctr", "res directory path and webserver instantiated :== " + res_directory_path);

        webServer.setToastHandler(&Application::show_toast);
        Application::write_log("Application::ctr", "ctr end.");

    }

    void Application::close_log()
    {
        if( Application::instance)
        {
            Application::instance->logFile->flush();
            Application::instance->logFile->close();
        }
    }

     void Application::write_log(const std::string methodName, const std::string message)
     {
       LOGE("%s():> %s", methodName.c_str(), message.c_str());

       if( Application::instance)
       {
           Application::instance->writeToFile(methodName + "():> " + message);
       }
     }

    void Application::check_jni_exception(JNIEnv *env, const std::string srcMethod, const std::string message)
    {
        if(env->ExceptionCheck() == JNI_TRUE)
        {
            Application::write_log(srcMethod, message );
            jthrowable throwable = env->ExceptionOccurred();
            env->ExceptionDescribe();

            env->ExceptionClear();
            jclass thrwbleClz = env->GetObjectClass(throwable);
            jmethodID getMessage = env->GetMethodID(thrwbleClz, "getMessage", "()Ljava/lang/String;");
            jstring message = (jstring)env->CallObjectMethod(throwable, getMessage);
            const char *mstr = env->GetStringUTFChars(message, NULL);
            const auto exception_msg = std::string{mstr};
            env->ReleaseStringUTFChars(message, mstr);
            env->DeleteLocalRef(message);
            env->DeleteLocalRef(thrwbleClz);

            Application::write_log(srcMethod, exception_msg);

            sleep(3);
            Application::close_log();
            sleep(4);

            env->Throw(throwable);
            //env->DeleteLocalRef(throwable);//in reality this call will never be reached.
        }
    }

    void Application::show_toast(const std::string message)
    {
        std::string srcMethod = "Application::showToast";
        Application::write_log(srcMethod, "start - message [> " + message );

        if(!Application::g_ctx.javaVM)
        {
            Application::write_log("Application::showToast", "JNI context not set on globalcontext variable. Exiting. end " );
            return;
        }

        //******************************************
        ALooper* mainLooper = Application::g_ctx.looper;
        int msgpipe[2];
        int readIdx = 0;
        int writeIdx = 1;

        if (pipe(msgpipe)) {
            Application::write_log(srcMethod, "Failed to create pipes. end " );
            return;
        }

        //fcntl(msgpipe[readIdx], F_SETFL, O_NONBLOCK);
        //fcntl(msgpipe[writeIdx], F_SETFL, O_NONBLOCK);

        std::function<int(int arg1, int arg2, void*)> callback = [&](int arg1, int arg2, void* data){

            JNIEnv* env;

            if ( Application::g_ctx.javaVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
                Application::write_log("Application::showToast", "Failed to create JNI environment. " );

                jint res = Application::g_ctx.javaVM->AttachCurrentThread( &env, NULL);
                if (JNI_OK != res && JNI_EDETACHED == res) {
                    Application::write_log("Application::showToast", "Failed to create JNI environment 2nd time by attaching current thread. end " );
                    return 0;
                }
            }

            jmethodID methodMakeText = env->GetStaticMethodID(Application::g_ctx.jniToastClz, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
            if(methodMakeText == NULL){
                Application::write_log("Application::showToast", "Failed to locate toast's makeText. end " );
                return 0;
            }

            Application::write_log(srcMethod, "Trying to acquired Application context from native side " );
            jobject  context = static_cast<jobject >(getJNIContext(env));
            Application::write_log(srcMethod, "Application context aquired" );

            Application::write_log(srcMethod, "Creating locale string reference." );
            jstring toastMessage = env->NewStringUTF(message.c_str());
            Application::check_jni_exception(env, srcMethod, "Exception occured creating locale string reference." );

            Application::write_log(srcMethod, "Constructing Toast object." );
            jobject toastobj = env->CallStaticObjectMethod(Application::g_ctx.jniToastClz, methodMakeText, context, toastMessage, 100);
            Application::check_jni_exception(env, srcMethod, "Exception occured constructing Toast object." );

            Application::write_log("Application::showToast", "Retrieving toast object's show method." );
            jmethodID showMethod = env->GetMethodID( Application::g_ctx.jniToastClz, "show", "()V");
            Application::check_jni_exception(env, srcMethod, "Exception occured retrieving toast object's show method." );

            Application::write_log("Application::showToast", "Invoking show method" );
            env->CallVoidMethod(toastobj, showMethod);
            Application::check_jni_exception(env, srcMethod, "Exception occured invoking show method." );

            Application::write_log("Application::showToast", "Deleting local string reference." );
            env->DeleteLocalRef(toastMessage);
            Application::write_log("Application::showToast", "end " );

            return 0;};

        ALooper_callbackFunc looperCb = Application::toastLooperHandler;  // *(static_cast<ALooper_callbackFunc*>(static_cast<void*>(&callback)));
        ALooper_addFd(mainLooper, msgpipe[readIdx],0, ALOOPER_EVENT_INPUT, looperCb , static_cast<void *>(&callback)); //looperCb

        char bte = 'j';
        write(msgpipe[writeIdx], &bte, 1);
        sleep(5);
    }

    int Application::toastLooperHandler(int arg1, int arg2, void* data)
    {
        Application::write_log("Application::toastLooperHandler", "start " );
        auto callback = *static_cast<std::function<int(int arg1, int arg2, void*)>*>(data);

        Application::write_log("Application::toastLooperHandler", "casted data and now invoking it " );
        callback(0,0, nullptr);
        Application::write_log("Application::toastLooperHandler", "end " );
        return 0;
    }

    void* Application::getJNIContext(JNIEnv *env) {
        jclass activityThreadCls = env->FindClass("android/app/ActivityThread");
        jmethodID currentActivityThread = env->GetStaticMethodID(activityThreadCls,
                                                                 "currentActivityThread",
                                                                 "()Landroid/app/ActivityThread;");
        jobject activityThreadObj =
                env->CallStaticObjectMethod(activityThreadCls, currentActivityThread);

        jmethodID getApplication =
                env->GetMethodID(activityThreadCls, "getApplication", "()Landroid/app/Application;");
        jobject context = env->CallObjectMethod(activityThreadObj, getApplication);
        return context;
    }

    Application& Application::get_instance(const std::string path)
     {
        Application::write_log("Application::get_instance", "address " );

       if(! Application::instance)
       {
           Application::write_log("Application::get_instance", "new instance being created " );
           Application::instance = new Application(path);
       }

        return *Application::instance;
     }

    void Application::cleanUp()
    {
        if(! Application::instance)
        {
            Application::write_log("Application::cleanUp", "instance already cleaned up. end" );
            return;
        }

        Application::write_log("Application::cleanUp", "instance is now being cleaned up. end 1" );
        delete Application::instance;
        Application::instance = nullptr;
        Application::write_log("Application::cleanUp", "instance is cleaned up. end 2" );
    }

    void Application::start(){
         Application::write_log("Application::start", "start");

         webServer.startServer();
         Application::write_log("Application::start", "end");
    }

    void Application::stop(){
         Application::write_log("Application::stop", "start");
         this->webServer.endServer();
         Application::write_log("Application::stop", "end");
    }

    Application::~Application(){
       wrsft::Application::write_log("Application::Destructor ~Application", "start");

       //if(this->webServer)
       {
          // delete this->webServer;
          // this->webServer = nullptr;
       }

        wrsft::Application::write_log("Application::Destructor ~Application", "end");
    }

    std::vector<std::string> Application::getFiles(std::string path) {
        std::vector<std::string> vec{};
        for (const auto & entry : fs::directory_iterator(path)) {

            //Application::write_log("Application::getFiles", entry.path());

            LOGE("%s():> %s", "Application::getFiles", entry.path().c_str());
            vec.push_back(entry.path());
        }
        return vec;
    }

    int Application::getStreamSize(std::string path) {

        LOGE("%s():> %s %s", "Application::getStreamSize", "start: ", path.c_str());
        auto size = Application::getFileSize(path);  //fs::file_size(path);

        LOGE("%s():> %d", "Application::getStreamSize. end: ", size);
        return size;
    }

    void Application::writeToFile(std::string content) {

        if(!logFile)
        {
            openOrCreateFile(log_directory_path);

            LOGI("%s():> %s", "Application::writeToFile", "file not yet openned for streaming!");
            //return;
        }

        if( ++writeCounter > 10000) {
            writeCounter = 0;

            if(fileSizeReachedMax(current_file))
            {
                auto newFullPathFile = generateNewFileName(log_directory_path);
                openFile(newFullPathFile);
            }
        }

        std::string dataTime = Application::getFormattedDateString();
        std::string formattedContent = dataTime+ ":: " + content + " \n";

        //LOGI("%s():> %s", "Application::writeToFile", "Writing to file.");

        (*logFile) << formattedContent;
        //logFile->write(formattedContent.c_str(), formattedContent.length() + 1);
        logFile->flush();

    }

    void Application::openOrCreateFile(std::string directory) {

        LOGE("%s():> %s", "Application::openOrCreateFile", directory.c_str());

        auto files = getFiles(directory);
        std::string foundFile = "";


        LOGE("%s():> %s", "Application::openOrCreateFile", "Search for existing file to write to.");

        for(auto& path : files )
        {
            auto maxSizeReached = fileSizeReachedMax(path);

            LOGE("%s():> %s", "Application::openOrCreateFile", "check file size against maximum");

            if(maxSizeReached) continue;

            LOGE("%s():> %s", "Application::openOrCreateFile", "will use the file named above :)");
            foundFile = path;
            break;
        }

        if(foundFile.length() == 0)
        {

            LOGE("%s():> %s", "Application::openOrCreateFile", "file not found creating new file for writing.");
            foundFile = generateNewFileName(directory);
            LOGE("%s():> %s", "Application::openOrCreateFile", "file name creation end reached.");
        }

        openFile(foundFile);
    }

    std::string Application::generateNewFileName(std::string directory) {
        using namespace std::literals;
        LOGE("%s():> %s", "Application::generateNewFileName", "file name creation end reached.");

        auto formatedDateTime = Application::getFormattedDateString() + ".log";

        fs::path _path{directory};
        fs::path fullPath = _path / formatedDateTime;

        LOGE("%s():> %s %s", "Application::generateNewFileName", "new file name is: ", fullPath.c_str());

        return fullPath;
    }

    std::string Application::getFormattedDateString()
    {
        using namespace std::literals;
       // LOGE("%s():> %s", "Application::getFormattedDateString", "computing date string format");
        //yyyyMMdd_HHmmss
        const std::chrono::time_point<std::chrono::system_clock> now =
                std::chrono::system_clock::now();

        const std::time_t t_c = std::chrono::system_clock::to_time_t(now - 24h);
        std::stringstream ss;

        ss << std::put_time(std::localtime(&t_c), "%Y%m%d_%H%M%S");

        return ss.str();
    }

    void Application::openFile(std::string fullPath)
    {
        LOGE("%s():> %s", "Application::openFile", fullPath.c_str(), ".  Start....");

        if(logFile && logFile->is_open())
        {
            LOGI("%s():> %s", "Application::openFile", "fileStream is not null and is already open. Closing it.");
            logFile->flush();
            logFile->close();
        }

        if(logFile)
        {
            LOGI("%s():> %s", "Application::openFile", "file stream is not null and ready to open or create file.");
            logFile->open(fullPath, fs::fstream::out | fs::fstream::app);
        }
        else
        {
            LOGI("%s():> %s", "Application::openFile", "file stream is not null and ready to open or create file.");
            logFile = std::make_unique<fs::fstream>(fullPath, fs::fstream::out | fs::fstream::app);
        }

        if(logFile->rdstate() == fs::fstream::failbit){
            LOGE("%s():> %s", "Application::openFile", "file stream failbit is set.. Resetting fileStream.");
            logFile.reset(nullptr);
        }

        LOGE("%s():> %s", "Application::openFile", fullPath.c_str(), ".  End....");
    }

    bool Application::fileSizeReachedMax(std::string fileFullPath) {

        LOGE("%s():> %s", "Application::fileSizeReachedMax", fileFullPath.c_str());


        if(! Application::doesFileExists(fileFullPath))
            return false;


        LOGE("%s():> %s", "Application::fileSizeReachedMax", "trying to access file size.");

        auto fileSize = getStreamSize(fileFullPath);
        LOGE("%s():> %s", "Application::fileSizeReachedMax", "file size acquired.");

        if(fileSize >= Application::FILE_SIZE)
            return true;

        return false;
    }

    bool Application::doesFileExists(std::string fileFullPath) {
        LOGE("%s():> %s", "Application::doesFileExists", fileFullPath.c_str());
        struct stat buffer;
        return (stat (fileFullPath.c_str(), &buffer) == 0);
    }

    size_t Application::getFileSize(std::string fileFullPath) {
        LOGE("%s():> %s", "Application::getFileSize", fileFullPath.c_str());
        struct stat st;
        stat(fileFullPath.c_str(), &st);
        auto size = static_cast<size_t>(st.st_size);
        LOGE("%s():> %s : %d", "Application::getFileSize", "has size: ", size);
        return  size;
    }

    std::string Application::getResDirPath(const std::string path) {

        fs::path rootdir =  { path};
        fs::path resdir = rootdir / "res";
        res_directory_path = resdir.string();
        return res_directory_path;
    }

    void Application::setJniContext(JavaVM *pVm, JNIEnv *env) {

        memset(&Application::g_ctx, 0, sizeof(Application::g_ctx));
        Application::g_ctx.javaVM = pVm;

        Application::g_ctx.looper = ALooper_forThread();

        if(!Application::g_ctx.looper)
        {
            Application::g_ctx.looper = ALooper_prepare(0);
        }

        ALooper_acquire(Application::g_ctx.looper);
        jclass  clz = env->FindClass("android/widget/Toast");
        Application::g_ctx.jniToastClz = static_cast<jclass >(env->NewGlobalRef( clz));
    }
}