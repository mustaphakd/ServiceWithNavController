//
// Created by musta on 11/29/2021.
//
//#include <__config>

#include "app.h"


// https://www.programcreek.com/cpp/?CodeExample=jni+onload
extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM * pVm, void* reserved)
{
    wrsft::Application::write_log("JNI_OnLoad", "start");
    JNIEnv* env;

    if ( pVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    //JNINativeMethod nm[2];
    //nm[0].name = "start_app";
    //nm[0].signature = "()V";//Ljava/lang/String;
    //nm[0].fnPtr = reinterpret_cast<void *>(start_app);
    //nm[1].name = "stop_app";
    //nm[1].signature = "()V";
    //nm[1].fnPtr = reinterpret_cast<void *>(stop_app);
    static const JNINativeMethod methods[] = {
            {"startapp", "(Ljava/lang/String;)V", reinterpret_cast<void*>(start_app)}, //Ljava/lang/String;
            {"stopapp", "()V", reinterpret_cast<void*>(stop_app)}
    };

    jclass cls = env->FindClass( "com/wrsft/servicewithnavcontroller/NativeWrapper");
    if (cls == nullptr) return JNI_ERR;
    // Register methods with env->RegisterNatives.
    int rc = env->RegisterNatives( cls, methods, sizeof(methods)/sizeof(JNINativeMethod));
    if (rc != JNI_OK) return rc;

    wrsft::Application::write_log("JNI_OnLoad", "end");

    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void start_app( JNIEnv *pEnv,  jobject pObj,  jstring directoryName) //,  jstring directoryName
{
    wrsft::Application::write_log("JNIEXPORT__start_app", "start" );

    /* Obtain a C-copy of the Java string */
    const char *str = pEnv->GetStringUTFChars(directoryName, 0);
    std::string folder {str};

   // wrsft::Application& _app = wrsft::Application::get_instance(folder);
   // _app.start();

    /* Now we are done with str */
    pEnv->ReleaseStringUTFChars(directoryName, str);

    wrsft::Application::write_log("JNIEXPORT__start_app", "end");
}

extern "C"
JNIEXPORT void stop_app( JNIEnv *pEnv, jobject pObj)
{
    wrsft::Application::write_log("JNIEXPORT__stop_app", "start");
   // wrsft::Application& _app = wrsft::Application::get_instance("");
    //_app.stop();
    wrsft::Application::write_log("JNIEXPORT__stop_app", "end");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wrsft_servicewithnavcontroller_NativeWrapper_startapp(JNIEnv *env, jobject thiz,
                                                               jstring directory) {
    // TODO: implement startapp()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wrsft_servicewithnavcontroller_NativeWrapper_stopapp(JNIEnv *env, jobject thiz) {
    // TODO: implement stopapp()
}