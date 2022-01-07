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
    uint32_t     events;      /* Epoll events * /
    epoll_data_t data;        /* User data variable * /
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
         *  consume may call register or unregister in the callback to add/remove client for further downstreaming such as when epoller
         *  detects data availabilty for clien consumption via pipes (in such scenario, control is returned to consumer to sendData payload).
         */
        void acceptConnection(int fd);

        /**
         * When data ready notification is sent to epoller, it iterates over each registered client fd and invoke sendData callback with the
         * client fd as parameter.
         */
        void sendData(int fd);

        void setNewRequestHandler(std::function<void(Channel&)> handler);
        void setExistingClientRequestHandler(std::function<void(Channel&)> handler);

        void startPolling();


    private:
        int epFd;
        int exitFdTrx[2];
        std::map<int, Channel> registeredChannels;
        const std::function<void(const std::string, const std::string)> logFunc;

        Epoller() = delete;
        Epoller& operator=(Epoller&) = delete;

        void setNonBlocking(int& fd);


        std::function<void(Channel&)> newRequestHandler;
        std::function<void(Channel&)> existingClientRequestHandler;

        int make_epoll_descriptor();
    };
}

#endif //SERVICEWITHNAVCONTROLLER_EPOLLER_H

namespace wrsft {

/* Set file descriptor to non-congested  */
    void Epoller::setNonBlocking(int& fd) {
        logFunc("Epoller::setNonBlocking", "start");
        int old_option = fcntl(fd, F_GETFL);
        int new_option = old_option | O_NONBLOCK;
        fcntl(fd, F_SETFL, new_option);
        logFunc("Epoller::setNonBlocking", "end");
    }

    Epoller::~Epoller() {
        logFunc("Epoller::~", "start");

        if(! epFd || epFd < 1)
        {
            logFunc("Epoller::~", "Epoller was never openned. Already closed.  end");
            return;
        }

        for (const auto& [key, value] : registeredChannels) { // could throw due to reference
            logFunc("Epoller::~", "unregistering fd: " + std::to_string(key));
            unregisterDescriptor(key);
        }

        ::close(epFd);
        epFd = -1;

        logFunc("Epoller::~", "end");
    }

    void Epoller::registerDescriptor(int fd) {
        logFunc("Epoller::registerDescriptor", "start");
        /**
         * EPOLLIN
The associated file is available for read(2) operations.
EPOLLOUT
The associated file is available for write(2) operations.
EPOLLRDHUP (since Linux 2.6.17)
Stream socket peer closed connection, or shut down writing half of connection. (This flag is especially useful for writing simple code to detect peer shutdown when using Edge Triggered monitoring.)
EPOLLPRI
There is urgent data available for read(2) operations.
EPOLLERR
Error condition happened on the associated file descriptor. epoll_wait(2) will always wait for this event; it is not necessary to set it in events.
EPOLLHUP
Hang up happened on the associated file descriptor. epoll_wait(2) will always wait for this event; it is not necessary to set it in events.
EPOLLET
         */
         Channel channel {0};
         channel.fd = fd;
         channel.is_exit_notifier = false;
         channel.is_primary = false;

        channel.epEvent = {0};
        channel.epEvent.events = EPOLLIN | EPOLLRDHUP |EPOLLET;
        //channel.epEvent.data.fd = fd;

        logFunc("Epoller::registerDescriptor", "Registering descriptor with kernel via epoll_Ctl");
        epoll_ctl(epFd, EPOLL_CTL_ADD, fd, &channel.epEvent );

        registeredChannels[fd] = channel;
        registeredChannels[fd].epEvent.data.ptr = &registeredChannels[fd];
        logFunc("Epoller::registerDescriptor", "end");
    }

    void Epoller::close() {
        logFunc("Epoller::close", "start");

        if(! epFd || epFd < 1)
        {
            logFunc("Epoller::close", "Epoller was never openned. Already closed.  end");
            return;
        }

        char bte = 'j';
        write(exitFdTrx[1], &bte, 1);
        sleep(5);

        logFunc("Epoller::close", "end");
    }

    void Epoller::sendData(int fd) {
        logFunc("Epoller::sendData", "start");
        //todo.  what to send ?
        logFunc("Epoller::sendData", "end");
    }

    void Epoller::unregisterDescriptor(int fd) {
        logFunc("Epoller::unregisterDescriptor", "start");

        auto channelItor = registeredChannels.find(fd);

        if (channelItor != registeredChannels.end()) {
            logFunc("Epoller::unregisterDescriptor", "fileDescription has been registered.  Found!");
            auto channel = channelItor->second ;

            epoll_ctl(epFd, EPOLL_CTL_DEL, channel.fd, &channel.epEvent);
            registeredChannels.erase(channelItor);

        } else {
            logFunc("Epoller::unregisterDescriptor", "fileDescription has not been registered.  Not Found!");
        }

        ::close(fd);
        logFunc("Epoller::unregisterDescriptor", "end");
    }

    void Epoller::acceptConnection(int fd) {  // w
        logFunc("Epoller::setexistingClientRequestHandler", "start");
        logFunc("Epoller::setexistingClientRequestHandler", "end");
    }

    void Epoller::setNewRequestHandler(std::function<void(Channel&)> handler) {
        logFunc("Epoller::setNewRequestHandler", "start");

        if(!handler)
        {
            logFunc("Epoller::setNewRequestHandler", "handler is not properly set. Aborting**** .end");
            abort();
        }

        newRequestHandler = handler;
        logFunc("Epoller::setNewRequestHandler", "end");
    }

    void Epoller::setExistingClientRequestHandler(std::function<void(Channel&)> handler) {
        logFunc("Epoller::setexistingClientRequestHandler", "start");

        if(!handler)
        {
            logFunc("Epoller::setexistingClientRequestHandler", "handler is not properly set. Aborting**** .end");
            abort();
        }

        existingClientRequestHandler = handler;
        logFunc("Epoller::setexistingClientRequestHandler", "end");
    }

    Epoller::Epoller(int fd,
                     const std::function<void(const std::string, const std::string)> loggerFunc): logFunc{loggerFunc}, epFd {make_epoll_descriptor()} {
        logFunc("Epoller::ctr", "start");

        Channel primaryChannel {0};
        primaryChannel.fd = fd;
        primaryChannel.is_primary = true;
        primaryChannel.is_exit_notifier = false;
        primaryChannel.epEvent = {0};
        primaryChannel.epEvent.events = EPOLLIN | EPOLLRDHUP |EPOLLET;
       // primaryChannel.epEvent.data.ptr = primaryChannel;

        epoll_ctl(epFd, EPOLL_CTL_ADD, fd, &primaryChannel.epEvent);

        logFunc("Epoller::ctr", "registering primary channel.");
        registeredChannels[fd] = primaryChannel;
        registeredChannels[fd].epEvent.data.ptr = &registeredChannels[fd];
        logFunc("Epoller::ctr", "end");
    }

    int Epoller::make_epoll_descriptor() {
        auto descriptor = epoll_create(10);
        return descriptor;
    }

    void Epoller::startPolling() {
        logFunc("Epoller::startPolling", "start");

        if (pipe(exitFdTrx)) {
            logFunc("Epoller::startPolling", "Failed to create pipes. end " );
            abort();
        }

        fcntl(exitFdTrx[0], F_SETFL, O_NONBLOCK);
        fcntl(exitFdTrx[1], F_SETFL, O_NONBLOCK);

        Channel pipedChannel {0};
        pipedChannel.fd = exitFdTrx[0];
        pipedChannel.is_primary = false;
        pipedChannel.is_exit_notifier = true;
        pipedChannel.epEvent = {0};
        pipedChannel.epEvent.events = EPOLLIN |EPOLLET;
        pipedChannel.epEvent.data.ptr = &pipedChannel;

        logFunc("Epoller::startPolling", "registering exiter pipe with kernel via epoll");
        epoll_ctl(epFd, EPOLL_CTL_ADD, exitFdTrx[0], &pipedChannel.epEvent);

        int eventSize = 20;
        epoll_event events[eventSize];
        int waitDurationMilli = 500;
        int readDescriptors;
        bool exit = false;

        for(;;)
        {
            readDescriptors = epoll_wait(epFd, events, eventSize, waitDurationMilli);

            if (readDescriptors == -1) {
                perror("epoll_wait()");
                logFunc("Epoller::startPolling", "epoll_wait failed.  exiting");
                exit = true;
            }

            for (int i = 0; i < readDescriptors; i++)
            {
                logFunc("Epoller::startPolling", "iterating over each descriptor for which event was raised.");
                // if some error ( // error case)
                if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN)))
                {
                    auto channel = reinterpret_cast<Channel*>(events[i].data.ptr);

                    if(channel && (channel->is_primary || channel->is_exit_notifier))
                    {
                        logFunc("Epoller::startPolling", "epoll error occured on either primary or exit-notifier descriptor.");
                        exit = true;
                        break;
                    }

                    logFunc("Epoller::startPolling", "epoll error occured on this file descriptor.");
                    unregisterDescriptor(events[i].data.fd);
                    continue;
                }
                else if ( reinterpret_cast<Channel*>(events[i].data.ptr)->is_primary)
                {
                    logFunc("Epoller::startPolling", "new connection is coming request thru primary fd.");
                    Channel primaryChannel = *reinterpret_cast<Channel*>(events[i].data.ptr);
                    newRequestHandler(primaryChannel);
                    /**
                     *
                     * // server socket; call accept as many times as we can
        for (;;) {
          struct sockaddr in_addr;
          socklen_t in_addr_len = sizeof(in_addr);
          int client = accept(sock, &in_addr, &in_addr_len);
          if (client == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              // we processed all of the connections
              break;
            } else {
              perror("accept()");
              return 1;
            }
          } else {
            printf("accepted new connection on fd %d\n", client);
            set_nonblocking(client);
            event.data.fd = client;
            event.events = EPOLLIN | EPOLLET;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &event) == -1) {
              perror("epoll_ctl()");
              return 1;
            }
          }
        }
                     */

                }
                else if ( reinterpret_cast<Channel*>(events[i].data.ptr)->is_exit_notifier)
                {
                    logFunc("Epoller::startPolling", "epoll_wait failed.  exiting");
                    // if exitFlag/Message
                    // Close method was invoked and called pipe write fd
                    // destructor should auto clean up
                    exit = true;
                    break;
                }
                else { // if existing client
                    logFunc("Epoller::startPolling", "existing client connected.");
                    Channel clientChannel = *reinterpret_cast<Channel*>(events[i].data.ptr);
                    existingClientRequestHandler(clientChannel);
                    // client socket; read as much data as we can
                    /*char buf[1024];
                    for (;;) {
                        ssize_t nbytes = read(events[i].data.fd, buf, sizeof(buf));
                        if (nbytes == -1) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                printf("finished reading data from client\n");
                                break;
                            } else {
                                perror("read()");
                                return 1;
                            }
                        } else if (nbytes == 0) {
                            printf("finished with %d\n", events[i].data.fd);
                            close(events[i].data.fd);
                            break;
                        } else {
                            fwrite(buf, sizeof(char), nbytes, stdout);
                        }
                    } */
                }
            }

            if(exit == true)
            {
                logFunc("Epoller::startPolling", "epoll_wait failed or exit was requested.  exiting");
                epoll_ctl(epFd, EPOLL_CTL_DEL, exitFdTrx[0], &pipedChannel.epEvent);
                break;
            }
        }

        logFunc("Epoller::startPolling", "end");
    }
}
