//#include <__config>
//
// Created by musta on 11/13/2021.
//
#include "app.h"

namespace fs = ghc::filesystem;

namespace wrsft {

    Application * Application::instance = nullptr;
    Application::Application(const std::string str):
        directory_path{str}, webServer{ WebServer<5>(&Application::write_log)}, logFile{nullptr}
    {

        Application::write_log("Application::ctr", "directory path and webserver instantiated :== " + directory_path);
    }

     void Application::write_log(const std::string methodName, const std::string message)
     {
       LOGE("%s():> %s", methodName.c_str(), message.c_str());

       if( Application::instance)
       {
           Application::instance->writeToFile(methodName + "():> " + message);
       }
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

        wrsft::Application::write_log("Application::Destructor ~Application", "stop");
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

        auto size = fs::file_size(path);

        LOGE("%s():> %s", "Application::getStreamSize", size);
        return size;
    }

    void Application::writeToFile(std::string content) {

        if(!logFile)
        {
            //openOrCreateFile()

            LOGE("%s():> %s", "Application::writeToFile", "file not yet openned for streaming!");
            return;
        }

        std::string dataTime = " ::";
        std::string formattedContent = dataTime + content + " \n";

        LOGE("%s():> %s", "Application::writeToFile", "Writing to file.");

        fs::fstream& streamer = *logFile;

          // formattedContent.c_str();

    }

    void Application::openOrCreateFile(std::string directory) {
        using namespace std::literals;

        LOGE("%s():> %s", "Application::openOrCreateFile", directory.c_str());

        auto files = getFiles(directory);
        std::string foundFile = "";


        LOGE("%s():> %s", "Application::openOrCreateFile", "Search for existing file to write to.");

        for(auto& path : files )
        {
            auto fileSize = getStreamSize(path);

            LOGE("%s():> %s", "Application::openOrCreateFile", path.c_str());

            if(fileSize >= Application::FILE_SIZE) continue;


            LOGE("%s():> %s", "Application::openOrCreateFile", "will use the file named above :)");
            foundFile = path;
            break;
        }

        if(foundFile.length() == 0)
        {

            LOGE("%s():> %s", "Application::openOrCreateFile", "file not found creating new file for writing.");
            //yyyyMMdd_HHmmss
            const std::chrono::time_point<std::chrono::system_clock> now =
                    std::chrono::system_clock::now();

            const std::time_t t_c = std::chrono::system_clock::to_time_t(now - 24h);
            std::stringstream ss;

            ss << std::put_time(std::localtime(&t_c), "%Y%m%d_%H%M%S");

            //fs::path _path {directory};
           // fs::path fullPath = _path /  ss.str();
           // foundFile = fullPath.c_str();

            //LOGE("%s():> %s", "Application::openOrCreateFile", "file name creation end reached");

        }


        LOGE("%s():> %s", "Application::openOrCreateFile", foundFile.c_str());

        // not createInstance fStream.  but first release existing if not null
        // see how to do it with unique_ptr

        if(logFile)
        {
            //logFile->close();
           // delete logFile;
            this->logFile = nullptr;
        }

        //logFile = new std::fstream (foundFile.c_str());


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