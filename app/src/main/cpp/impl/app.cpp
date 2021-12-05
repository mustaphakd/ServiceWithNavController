//#include <__config>
//
// Created by musta on 11/13/2021.
//
#include "app.h"


namespace wrsft {

    Application * Application::instance = nullptr;
    Application::Application(const std::string str):
        directory_path{str}, webServer{ WebServer<5>(&Application::write_log)}
    {

        Application::write_log("Application::ctr", "directory path and webserver instantiated");
    }

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

        wrsft::Application::write_log("Application::Destructor ~Application", "stop");
    }

}
/*
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
*/