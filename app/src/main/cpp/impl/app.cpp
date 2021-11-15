#include <__config>
//
// Created by musta on 11/13/2021.
//
#include "app.h"



namespace wrsft {

    Application * Application::instance = nullptr;
    Application::Application(const std::string str): directory_path{str} {}

     void Application::write_log(const std::string methodName, const std::string message)
     {
       LOGE("%s():> %s", methodName.c_str(), message.c_str());
     }

     Application& Application::get_instance(const std::string path)
     {
       if(! Application::instance)
       {
         Application::instance = new Application(path);
       }

       return *Application::instance;
     }

     void Application::start(){}
     void Application::stop(){}

}

// https://www.programcreek.com/cpp/?CodeExample=jni+onload
extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM * pVm, __unused void* reserved)
{
    wrsft::Application::write_log("JNI_OnLoad", "start");
     JNIEnv* env;

     if ( pVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
       return JNI_ERR;
     }

     /*JNINativeMethod nm[2];
     nm[0].name = "start_app";
     nm[0].signature = "(Ljava/lang/String;)V";
     nm[0].fnPtr = reinterpret_cast<void *>(start_app);
     nm[1].name = "stop_app";
     nm[1].signature = "()V";
     nm[1].fnPtr = reinterpret_cast<void *>(stop_app); */
    static const JNINativeMethod methods[] = {
            {"start_app", "(Ljava/lang/String;)Ljava/lang/Void;", reinterpret_cast<void*>(start_app)},
            {"stop_app", "()Ljava/lang/Void;", reinterpret_cast<void*>(stop_app)},
    };

    jclass cls = env->FindClass( "com/wrsft/servicewithnavcontroller/NativeWrapper");
    if (cls == nullptr) return JNI_ERR;
     // Register methods with env->RegisterNatives.
    int rc = env->RegisterNatives( cls, methods, sizeof(methods)/sizeof(JNINativeMethod));
    if (rc != JNI_OK) return rc;

     wrsft::Application::write_log("JNI_OnLoad", "end");

    return JNI_VERSION_1_6;
}

void start_app(__unused JNIEnv *pEnv, __unused jobject pObj, __unused jstring directoryName)
{
    wrsft::Application::write_log("start_app", "start" );
    wrsft::Application::write_log("start_app", "end");
}
extern "C"
void stop_app(__unused JNIEnv *pEnv, jobject pObj)
{
    wrsft::Application::write_log("stop_app", "start");
    wrsft::Application::write_log("stop_app", "end");
}
/*
extern "C"
JNIEXPORT jobject JNICALL
Java_com_wrsft_servicewithnavcontroller_MainActivity_start_1app(JNIEnv *env, jobject thiz) {
    // TODO: implement start_app()
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_wrsft_servicewithnavcontroller_MainActivity_stop_1app(JNIEnv *env, jobject thiz) {
    // TODO: implement stop_app()
} */