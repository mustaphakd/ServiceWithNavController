//
// Created by musta on 11/29/2021.
//
/*
#include "webServer.h"

namespace wrsft {

    template<int maxClient>
    void WebServer<maxClient>::startServer() {
        logfunc("WebServer::startServer", "start");
        if (running) return;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int result = pthread_create( &worker, &attr, this->run, NULL);

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
    } */

    /*template<int maxClient>
    WebServer<maxClient>::~WebServer()= default;

    template<int maxClient>
    void WebServer<maxClient>::setSeamer(Streamer& streamer){
        logfunc("WebServer::setStreamer", "start - end");
    } */
//}