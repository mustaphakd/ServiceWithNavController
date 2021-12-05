//
// Created by musta on 11/29/2021.
//

#ifndef SERVICEWITHNAVCONTROLLER_WEBSERVER_H
#define SERVICEWITHNAVCONTROLLER_WEBSERVER_H

#include <functional>
#include <string>
#include <pthread.h>
#include <unistd.h>

#define MAX_CLIENT 5

//class Streamer;
namespace wrsft {

    typedef std::function<void (const std::string, const std::string)> LoggerType;

    template <int maxClient = MAX_CLIENT>
    class WebServer {

    public:
        WebServer(const LoggerType& logger): logfunc{logger}, running{0}, exit{0}{
            logfunc("WebServer::ctr", "start - end");
        }

        void startServer();
        void endServer();
        bool  isServerRunning() {
            logfunc("WebServer::isServerRunning", "isRunning ?? =>" + running);
            return running == 1;
        }

        void waitUntilStopped(){
            while(running){
                logfunc("WebServer::waitUntilStopped", "will sleep for 3 seconds.");
                sleep(3);
            }
        }
        void run();
        static void runWrapper(WebServer& server);
        void sendDataToClients();
        //~WebServer();
       // void setStreamer(Streamer& streamer);



    private:
        int _maxCLient = maxClient;
        int running;
        int exit;
        const LoggerType logfunc;
        pthread_t worker;


    };
}

#endif //SERVICEWITHNAVCONTROLLER_WEBSERVER_H


namespace wrsft {

    typedef void * (*THREADFUNCPTR)(void *);
    template<int maxClient>
    void WebServer<maxClient>::startServer() {
        logfunc("WebServer::startServer", "start");
        if (running) return;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int result = pthread_create( &worker, &attr, (THREADFUNCPTR) &WebServer<maxClient>::runWrapper, this);

        logfunc("WebServer::startServer", "pthread creation result: " + std::to_string(result));

        running = 1;
        logfunc("WebServer::startServer", "end");
    }

    template<int maxClient>
    void WebServer<maxClient>::endServer(){
        logfunc("WebServer::endServer", "start");
        if(!running)return;

        exit = 1;

        logfunc("WebServer::endServer", "Wait until thread stops");
        waitUntilStopped();
        logfunc("WebServer::endServer", "end");
    }

    template<int maxClient>
    void WebServer<maxClient>::sendDataToClients(){
        logfunc("WebServer::sendDataToClien", "start - end");
    }

    template<int maxClient>
    void WebServer<maxClient>::run() {
        logfunc("WebServer<maxClient>::run", "start");

        while (true)
        {
            logfunc("WebServer<maxClient>::run", "running....");

            if(exit){
                logfunc("WebServer<maxClient>::run", "exit requested breaking out.");
                exit = 0;
                break;
            }
        }

        running = 0;
        pthread_exit(0);
        logfunc("WebServer<maxClient>::run", "end");
        // raise condition here just in case
    }

    template<int maxClient>
    void WebServer<maxClient>::runWrapper(WebServer &server) { //(WebServer &server)
        server.run();
    }

    /*template<int maxClient>
    WebServer<maxClient>::~WebServer()= default;

    template<int maxClient>
    void WebServer<maxClient>::setSeamer(Streamer& streamer){
        logfunc("WebServer::setStreamer", "start - end");
    } */
}