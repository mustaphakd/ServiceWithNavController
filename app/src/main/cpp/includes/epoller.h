//
// Created by musta on 1/2/2022.
//

#ifndef SERVICEWITHNAVCONTROLLER_EPOLLER_H
#define SERVICEWITHNAVCONTROLLER_EPOLLER_H

#include <sys/epoll.h>
#include <map>
#include <string>

namespace wrsft {

    /**
     * typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      / * Epoll events * /
    epoll_data_t data;         * User data variable * /
};
     */

    struct Channel {
        int fd;
        epoll_event epEvent;
        bool is_primary;
        bool is_exit_notifier;
        void* data;
    };

    class Epoller {
    public:
        Epoller(int fd, const std::function<void(const std::string, const std::string)> loggerFunc);
        ~Epoller();

        void registerDescriptor(int fd);
        void unregisterDescriptor(int fd);
        void close();

        /**
         *  runtime invokes this callback provided by the cosummer when a request is sent to server.
         *  consume may call register or unregister in the callback to add/remove client for further downstreaming such as when weakPoller
         *  detects data availabilty for clien consumption via pipes (in such scenario, control is returned to consumer to sendData payload).
         */
        void acceptConnection(int fd);

        /**
         * When data ready notification is sent to weakPoller, it iterates over each registered client fd and invoke sendData callback with the
         * client fd as parameter.
         */
        void sendData(int fd);

        void setNewRequestHandler(std::function<void(Channel&)> handler);
        void setExistingClientRequestHandler(std::function<void(Channel&)> handler);

        void startPolling();


    private:
        int epFd;
        int serverFd;
        int exitFdTrx[2];
        std::map<int, std::shared_ptr<Channel>> registeredChannels;
        const std::function<void(const std::string, const std::string)> logFunc;

        Epoller() = delete;
        Epoller& operator=(Epoller&) = delete;

        void setNonBlocking(int& fd);


        std::function<void(Channel&)> newRequestHandler;
        std::function<void(Channel&)> existingClientRequestHandler;

        int make_epoll_descriptor();

        std::shared_ptr<Channel> findChannel(int fd);
    };
}

#endif //SERVICEWITHNAVCONTROLLER_EPOLLER_H

