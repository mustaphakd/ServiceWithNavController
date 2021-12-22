//
// Created by musta on 11/29/2021.
//

#ifndef SERVICEWITHNAVCONTROLLER_WEBSERVER_H
#define SERVICEWITHNAVCONTROLLER_WEBSERVER_H

#include <functional>
#include <stdio.h>
#include <string>
#include <sstream>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <linux/in.h>

#include <morris/ifaddrs.h>
//#include <netinet/in.h> // makes inet_ntop disappear
#include <fs/filesystem.hpp>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <netdb.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

namespace fs = ghc::filesystem;

#define MAX_CLIENT 5

//class Streamer;
namespace wrsft {

    typedef std::function<void (const std::string, const std::string)> LoggerType;

    template <int maxClient = MAX_CLIENT>
    class WebServer {

    public:
        WebServer(const LoggerType& logger, const std::string resDirectory): logfunc{logger}, res_directory{resDirectory}, running{0}, exit{0}{
            logfunc("WebServer::ctr", "start ***  " + res_directory + " *** - end");
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
        const std::string res_directory;
        pthread_t worker;

        std::vector<char> readBinContent(std::string fullPath, size_t& contentSize);
        std::string readFileContent(std::string fullPath);
        void printNtwrkInterface();

        bool check_wireless(const char *name, char aProtocol[16]);
        std::string get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);

        void printMacAddress();

        std::string printSaFamily(const sockaddr *sa);
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
        auto logCounter = 0;

        printNtwrkInterface();
        printMacAddress();

        while (true)
        {
            if(logCounter++ >= 5000000000)
            {
                logfunc("WebServer<maxClient>::run", "running....");
                logCounter = -100;
            }

           // logfunc("WebServer<maxClient>::run", "running....");

            if(exit){
                logfunc("WebServer<maxClient>::run", "exit requested breaking out.");
                exit = 0;
                break;
            }
        }

        running = 0;
        logfunc("WebServer<maxClient>::run", "end");
        pthread_exit(0);
        // raise condition here just in case
    }

    template<int maxClient>
    void WebServer<maxClient>::runWrapper(WebServer &server) { //(WebServer &server)
        server.run();
    }

    template<int maxClient>
    std::vector<char> WebServer<maxClient>::readBinContent(std::string fullPath, size_t& contentSize) {

        fs::ifstream ifs(fullPath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

        if(ifs.fail())
        {
            //loghere
        }

        fs::ifstream::pos_type fileSize = ifs.tellg();
        contentSize = static_cast<size_t >(fileSize);

        if (fileSize < 0)                             //<--- ADDED
        return std::vector<char> {};                     //<--- ADDED

        ifs.seekg(0, std::ios::beg);

        std::vector<char> bytes(fileSize);
        ifs.read(&bytes[0], fileSize);


        if(ifs.fail() && !ifs.eof())
        {
            //loghere
        }

        return bytes;
    }

    template<int maxClient>
    std::string WebServer<maxClient>::readFileContent(std::string fullPath) {

        size_t fileSize;
        auto bytes = readBinContent(fullPath, &fileSize);
        return string(&bytes[0], fileSize);
    }

    template<int maxClient>
    void WebServer<maxClient>::printNtwrkInterface() {
        logfunc("WebServer<maxClient>::printNtwrkInterface", "start");
        struct ifaddrs *ifaddr, *ifa;

        if (getifaddrs(&ifaddr) == -1) {
            logfunc("WebServer<maxClient>::printNtwrkInterface", "Error.  end");
            return;
        }

       /* std::string s;
        s.reserve(100); */

        char s[100];

        /* Walk through linked list, maintaining head pointer so we
           can free list later */
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            char protocol[IFNAMSIZ]  = {0};
            char *macp;

            if (ifa->ifa_addr == NULL ) continue; // ||  ifa->ifa_addr->sa_family != AF_PACKET


            std::stringstream text;

            text << "interface " << ifa->ifa_name << " " << "\tFamily: " << ifa->ifa_addr ? ifa->ifa_addr->sa_family : -1;

            if (check_wireless(ifa->ifa_name, protocol)) {
                text << " wireless protocol: " << protocol;
            }

            auto s1 = get_ip_str(ifa->ifa_addr, s, 99);

            text << "\taddress: " << s << " ------" << s1;
            //s = "";

            if(ifa->ifa_netmask) {
                memset(s, 0, 100);
                s1 = get_ip_str(ifa->ifa_netmask, s, 99); //const_cast<char *>(s.c_str())
                text << "\tmask: " << s << " ------" << s1;
            }

//https://stackoverflow.com/questions/6762766/mac-address-with-getifaddrs ios osx are different
            struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
            int i;
            int len = 0;
            for(i = 0; i < 6; i++)
                len+=sprintf(macp+len,"%02X%s",s->sll_addr[i],i < 5 ? ":":"");
            text << "\n\tmac: " << macp << " -" ;


            logfunc("WebServer<maxClient>::printNtwrkInterface", text.str());
            //s = "";
            memset(s, 0, 100);
        }

        freeifaddrs(ifaddr);
        logfunc("WebServer<maxClient>::printNtwrkInterface", "end");
    }

    template<int maxClient>
    std::string WebServer<maxClient>::printSaFamily(const struct sockaddr *sa)
    {
        logfunc("WebServer<maxClient>::printSaFamily", "end");
        std::stringstream text;

        if(!sa)
        {
            text << "unknown family";
        }
        else
        {
            text << "family: " << sa->sa_family;

            switch(sa->sa_family)
            {

            }
        }

        logfunc("WebServer<maxClient>::printSaFamily", text.str() + ".  end");
        return text.str();
    }

    template<int maxClient>
    bool WebServer<maxClient>::check_wireless(const char* ifname, char* protocol) {
        int sock = -1;
        struct iwreq pwrq;
        memset(&pwrq, 0, sizeof(pwrq));
        strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            return 0;
        }

        if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) {
            if (protocol) strncpy(protocol, pwrq.u.name, IFNAMSIZ);
            close(sock);
            return 1;
        }

        close(sock);
        return 0;
    }

    template<int maxClient>
    std::string
    WebServer<maxClient>::get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
        logfunc("WebServer<maxClient>::get_ip_str", "start");
        switch(sa->sa_family) {
            case AF_INET:
                logfunc("WebServer<maxClient>::get_ip_str", "AF_INET");
                inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                          s, maxlen);
                break;

            case AF_INET6:
                logfunc("WebServer<maxClient>::get_ip_str", "AF_INET6");
                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                          s, maxlen);
                break;

            default:
                logfunc("WebServer<maxClient>::get_ip_str", "default");
                strncpy(s, "Unknown AF", maxlen);
                return "Unknown AF"; //NULL;
        }

        logfunc("WebServer<maxClient>::get_ip_str", "end");
        return s;
    }

    template<int maxClient>
    void WebServer<maxClient>::printMacAddress() {
        char buf[8192] = {0};
        struct ifconf ifc = {0};
        struct ifreq *ifr = NULL;
        int sck = 0;
        int nInterfaces = 0;
        int i = 0;
        char ip[INET6_ADDRSTRLEN] = {0};
        char macp[19];
        struct ifreq *item;
        struct sockaddr *addr;


        logfunc("WebServer<maxClient>::printMacAddress", "start");

        /* Get a socket handle. */
        sck = socket(PF_INET, SOCK_DGRAM, 0);
        if(sck < 0)
        {
            logfunc("WebServer<maxClient>::printMacAddress", " Error with socket. ... end");
            return;
        }

        /* Query available interfaces. */
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
        {
            logfunc("WebServer<maxClient>::printMacAddress", " Error with ioctl(SIOCGIFCONF). ... end");
            return;
        }

        /* Iterate through the list of interfaces. */
        ifr = ifc.ifc_req;
        nInterfaces = ifc.ifc_len / sizeof(struct ifreq);


        std::stringstream text;

        for(i = 0; i < nInterfaces; i++)
        {
            item = &ifr[i];

            addr = &(item->ifr_addr);

            /* Get the IP address*/
            if(ioctl(sck, SIOCGIFADDR, item) < 0)
            {
                logfunc("WebServer<maxClient>::printMacAddress", " Error with ioctl(OSIOCGIFADDR). ... end");
            }

            if (inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), ip, sizeof ip) == NULL)
            {
                logfunc("WebServer<maxClient>::printMacAddress", " Error with inet_ntop. ... end");
                continue;
            }

            /* Get the MAC address */
            if(ioctl(sck, SIOCGIFHWADDR, item) < 0) {
                logfunc("WebServer<maxClient>::printMacAddress", " Error with ioctl(SIOCGIFHWADDR). ... end");
                return;
            }

            /* display result */
            text.str(std::string());

            sprintf(macp, " %02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned char)item->ifr_hwaddr.sa_data[0],
                    (unsigned char)item->ifr_hwaddr.sa_data[1],
                    (unsigned char)item->ifr_hwaddr.sa_data[2],
                    (unsigned char)item->ifr_hwaddr.sa_data[3],
                    (unsigned char)item->ifr_hwaddr.sa_data[4],
                    (unsigned char)item->ifr_hwaddr.sa_data[5]);

            printf("%s %s ", ip, macp);
            text << ip << " --->" << macp << "\n";
            logfunc("WebServer<maxClient>::printMacAddress", text.str());

        }


        logfunc("WebServer<maxClient>::printMacAddress", "end");
    }

    /*template<int maxClient>
    WebServer<maxClient>::~WebServer()= default;

    template<int maxClient>
    void WebServer<maxClient>::setSeamer(Streamer& streamer){
        logfunc("WebServer::setStreamer", "start - end");
    } */
}