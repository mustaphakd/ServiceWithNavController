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
#include "stringHelper.h"
#include "sslHandler.h"
#define MAX_CLIENT 5

namespace fs = ghc::filesystem;
using namespace wrsft;

extern "C++" std::string split(std::string , std::string );

//class Streamer;
namespace wrsft {

    //typedef std::function<void (const std::string, const std::string)> LoggerType;
    typedef std::function<void (const std::string)> ToasterType;
    typedef std::function<void (const std::string)> NotificationType;

    template <int maxClient = MAX_CLIENT>
    class WebServer : public SSLHandler<8088>{ //SSLHandler<"cert.pem", "key.pem">

    public:
        WebServer(const LoggerType& logger, const std::string resDirectory):  res_directory{resDirectory}, running{0}, exit{0}
         , SSLHandler<8088>(logger, resDirectory)
        {
            logfunc("WebServer::ctr", "start ***  " + res_directory + " *** - end");
            toasterfunc = nullptr;
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
        void setToastHandler(const ToasterType handler);
        void setNotificationHandler(const NotificationType handler);

        //~WebServer();
       // void setStreamer(Streamer& streamer);



    private:
       // int _maxCLient = maxClient;
        int running;
        int exit;
        ToasterType toasterfunc;
        NotificationType notificationfunc;
        const std::string res_directory;
        pthread_t worker;

        std::string ip4Address;
        std::string ip6Address;
        std::string ip4MAC;
        std::string ip6MAC;

        std::vector<char> readBinContent(std::string fullPath, size_t& contentSize);
        std::string readFileContent(std::string fullPath);
        void printNtwrkInterface();
        void showToast(const std::string message);
        void showNotification(const std::string message);

        bool check_wireless(const char *name, char aProtocol[16]);
        std::string get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);

        void printMacAddress();

        std::string printSaFamily(const sockaddr *sa);

        void storeNetworkDeviceInfo(NetworkDevice device, std::string str, std::string searchToken, std::string& addressStorage, std::string& macStorage );
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
        stop_handler();

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

        //while (true)
        //{
            if(logCounter++ >= 4200000000)
            {
                logfunc("WebServer<maxClient>::run", "running....");
                logCounter = -100;
            }

            run_handler();
           // logfunc("WebServer<maxClient>::run", "running....");

            if(exit){
                logfunc("WebServer<maxClient>::run", "exit requested breaking out.");
                exit = 0;
                //break;
            }
        //}

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
            text << "interface[> " << ifa->ifa_name << " ----- " << "  Family: " << ifa->ifa_addr ? static_cast<unsigned short>(ifa->ifa_addr->sa_family) : -1;

            if (check_wireless(ifa->ifa_name, protocol)) {
                text << " wireless protocol: " << protocol;
            }

            auto s1 = get_ip_str(ifa->ifa_addr, s, 99);
            text << " -----\taddress[> " << s1 ; //<< " ------" << s1;

//https://stackoverflow.com/questions/6762766/mac-address-with-getifaddrs ios osx are different
            struct sockaddr_ll *sckaddr = (struct sockaddr_ll*)ifa->ifa_addr;
            int i;
            int len = 0;
            for(i = 0; i < 6; i++)
                len+=sprintf(macp+len,"%02X%s",sckaddr->sll_addr[i],i < 5 ? ":":"");
            text << " -----\tmac[> " << macp ;


            if(ifa->ifa_netmask) {
                memset(s, 0, 100);
                s1 = get_ip_str(ifa->ifa_netmask, s, 99); //const_cast<char *>(s.c_str())
                text << " -----\tmask[> " << s1 ; //<< " ------" << s1;
            }

            auto ntwrkinfo = text.str();
            storeNetworkDeviceInfo(NetworkDevice::wlan, ntwrkinfo, ".", ip4Address, ip4MAC);
            storeNetworkDeviceInfo(NetworkDevice::wlan, ntwrkinfo, ":", ip6Address, ip6MAC);

            logfunc("WebServer<maxClient>::printNtwrkInterface", ntwrkinfo);
            memset(s, 0, 100);
        }

        freeifaddrs(ifaddr);

        std::stringstream nicInfo;
        nicInfo << "Ip4 address: " << ip4Address << "\tIp4 MAC: " << ip4MAC ;
        nicInfo << "\nIp6 address: " << ip6Address << "\tIp6 MAC: " << ip6MAC ;
        nicInfo << "\nAccess your feed using https://"<< ip4Address <<":8088";
        logfunc("\n\n", nicInfo.str());

        char urlBuffer [100];
        auto n = sprintf (urlBuffer, "https://%s:8088", ip4Address.c_str());
        std::string notificationMessage {urlBuffer};
        showNotification(urlBuffer);

        memset(urlBuffer, 0, 100);
        sprintf (urlBuffer, "\nAccess your feed using:\nhttps://%s:8088", ip4Address.c_str());
        std::string toastMessage {urlBuffer};
        showToast(toastMessage);

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
                case AF_PACKET:
                    text << " => AF_PACKET";
                    break;
                default:
                    text << " => not accounted for.";
                    break;
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
                inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
                break;

            case AF_INET6:
                logfunc("WebServer<maxClient>::get_ip_str", "AF_INET6");
                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
                break;

            default:
                logfunc("WebServer<maxClient>::get_ip_str", "default = " + sa->sa_family);
                strncpy(s, "Unknown AF", maxlen);
                // "Unknown AF"; //NULL;
                break;
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

    template<int maxClient>
    void WebServer<maxClient>::storeNetworkDeviceInfo(wrsft::NetworkDevice device, std::string str,
                                                      std::string searchToken, std::string &addressStorage,
                                                      std::string &macStorage) {


        logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "start");

        std::string deviceStr = NetworkDeviceToString(device);


        logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "searching for device type: " + deviceStr);

        if(str.find(deviceStr) == std::string::npos) return;

        std::string delimiter = "-----";

        logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "spliting string into segments");
        std::vector<std::string> segments = split(str , delimiter);

        auto unknowTokenFound = false;
        auto correctIpAddressFound = false;


        logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "source string split into segments.");

        for(auto& segment : segments)
        {
            logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "processing segment: " + segment);

            if(segment.find("address") != std::string::npos)
            {
                logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "start address segment processing.");
                std::vector<std::string> addressPath = wrsft::split(segment , "[>");
                auto unknowToken = addressPath[1].find("Unknown");
                unknowTokenFound = unknowToken != std::string::npos;

                if( ( ! unknowTokenFound) &&
                        (addressPath[1].find(searchToken) != std::string::npos))
                {
                    addressStorage = addressPath[1];
                    correctIpAddressFound = true;
                    logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "address for type " + deviceStr + " : " + addressStorage);
                }

                logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "end address segment processing.");
                continue;
            }

            if(segment.find("mac") != std::string::npos &&
                    (correctIpAddressFound || unknowTokenFound))
            {
                logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "start mac segment processing.");
                std::vector<std::string> macPath = wrsft::split(segment , "[>");

                if(macPath[1].find("00:00:00:00:02:00") == std::string::npos)
                {
                    macStorage = macPath[1];
                    logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "mac for type " + deviceStr + " : " + macStorage);
                }
                logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "end mac segment processing.");
            }
        }


        logfunc("WebServer<maxClient>::storeNetworkDeviceInfo", "end");

       /* auto start = 0U;
        auto end = s.find(delim);
        while (end != std::string::npos)
        {
            std::cout << s.substr(start, end - start) << std::endl;
            start = end + delim.length();
            end = s.find(delim, start);
        }

        std::cout << s.substr(start, end);*/


    }

    template<int maxClient>
    void
    WebServer<maxClient>::setToastHandler(const std::function<void(const std::string)> handler) {
        logfunc("WebServer<maxClient>::setToastHandler", "start");
        toasterfunc = handler;
        logfunc("WebServer<maxClient>::setToastHandler", "end");
    }

    template<int maxClient>
    void WebServer<maxClient>::showToast(const std::string message) {
        logfunc("WebServer<maxClient>::showToast", "start => " + message);

        if(toasterfunc)
        {
            logfunc("WebServer<maxClient>::showToast", "sending message...");
            auto copy = message;
            toasterfunc(copy);
        }

        logfunc("WebServer<maxClient>::showToast", "end");
    }

    template<int maxClient>
    void
    WebServer<maxClient>::setNotificationHandler(const std::function<void(const std::string)> handler) {
        logfunc("WebServer<maxClient>::setNotificationHandler", "start");
        notificationfunc = handler;
        logfunc("WebServer<maxClient>::setNotificationHandler", "end");
    }


    template<int maxClient>
    void WebServer<maxClient>::showNotification(const std::string message) {
        logfunc("WebServer<maxClient>::showNotification", "start => " + message);

        if(notificationfunc)
        {
            logfunc("WebServer<maxClient>::showNotification", "sending message...");
            auto copy = message;
            notificationfunc(copy);
        }

        logfunc("WebServer<maxClient>::showNotification", "end");
    }

    /*template<int maxClient>
    WebServer<maxClient>::~WebServer()= default;

    template<int maxClient>
    void WebServer<maxClient>::setSeamer(Streamer& streamer){
        logfunc("WebServer::setStreamer", "start - end");
    } */
}