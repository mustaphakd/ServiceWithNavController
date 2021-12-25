//
// Created by musta on 12/24/2021.
//

#ifndef SERVICEWITHNAVCONTROLLER_GLOBALFIX_H
#define SERVICEWITHNAVCONTROLLER_GLOBALFIX_H

#include <android/looper.h>

typedef struct jni_context { //typedef
    JavaVM  *javaVM;
    jclass   jniToastClz;
    ALooper *looper;
    jobject  jniToastObj;
} JniContext;
#endif //SERVICEWITHNAVCONTROLLER_GLOBALFIX_H
