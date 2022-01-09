//
// Created by musta on 1/7/2022.
//

#include <fcntl.h>
#include <unistd.h>
#include "epoller.h"


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

        newRequestHandler = nullptr;
        existingClientRequestHandler = nullptr;
        const int size = registeredChannels.size();
        int fdArray[size];
        int index = 0;

        for (auto iter = registeredChannels.crbegin(); iter != registeredChannels.crend(); iter++) { // could throw due to reference
            auto key = iter->first;
            logFunc("Epoller::~", "storing value for unregistering fd: " + std::to_string(key));
            fdArray[index++] = key;
        }

        for(auto idx : fdArray)
        {
            logFunc("Epoller::~", "unregistering fd: " + std::to_string(idx));
            unregisterDescriptor(idx);
        }

        logFunc("Epoller::~", "Closing epoller file-description...");
        ::close(epFd);
        logFunc("Epoller::~", "Closing epoller file-description...done.");
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
        auto channel = std::make_shared<Channel>();
        channel->fd = fd;
        channel->is_exit_notifier = false;
        channel->is_primary = false;

        channel->epEvent = {0};
        channel->epEvent.events = EPOLLIN | EPOLLRDHUP |EPOLLET;
        //channel.epEvent.data.fd = fd;

        logFunc("Epoller::registerDescriptor", "Registering descriptor with kernel via epoll_Ctl");
        epoll_ctl(epFd, EPOLL_CTL_ADD, fd, &channel->epEvent );

        registeredChannels[fd] = channel;
        registeredChannels[fd]->epEvent.data.fd = fd;
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
        sleep(1);

        logFunc("Epoller::close", "end");
    }

    void Epoller::sendData(int fd) {
        logFunc("Epoller::sendData", "start");
        //todo.  what to send ?
        logFunc("Epoller::sendData", "end");
    }

    std::shared_ptr< Channel>  Epoller::findChannel(int fd) {
        logFunc("Epoller::findChannel", "start");

        auto channelItor = registeredChannels.find(fd);

        if (channelItor != registeredChannels.end()) {
            logFunc("Epoller::findChannel",
                    "fileDescription has been registered.  Found!");
            return channelItor->second;
        }

        logFunc("Epoller::findChannel", "end");
        return nullptr;
    }

    void Epoller::unregisterDescriptor(int fd) {
        logFunc("Epoller::unregisterDescriptor", "start");


        auto channel  = std::make_shared<Channel>(*findChannel(fd));

        if (channel) {
            logFunc("Epoller::unregisterDescriptor", "fileDescription has been registered.  Found!");

            auto localFd = channel->fd;
            epoll_event localEvents {0};
            localEvents.data = {0};
            localEvents.data.ptr = nullptr;
            localEvents.data.fd = localFd;

            localEvents.events = EPOLLIN |EPOLLET ;// channel->epEvent.events; // ((EPOLLIN |EPOLLET) &&  make copies before deleting fds from epoll ctl

            channel.reset();

            epoll_ctl(epFd, EPOLL_CTL_DEL, localFd, nullptr);
           // sleep(3);
            registeredChannels.erase(localFd);

        } else {
            logFunc("Epoller::unregisterDescriptor", "fileDescription has not been registered.  Not Found!");
        }


        ::close(fd);

        sleep(10);
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
                     const std::function<void(const std::string, const std::string)> loggerFunc): serverFd {fd}, logFunc{loggerFunc}, epFd {make_epoll_descriptor()} {
        logFunc("Epoller::ctr", "start - fd is: " + std::to_string(fd));

        std::shared_ptr<Channel> primaryChannel = std::make_shared<Channel>();
        primaryChannel->fd = fd;
        primaryChannel->is_primary = true;
        primaryChannel->is_exit_notifier = false;
        primaryChannel->epEvent = {0};
        primaryChannel->epEvent.events = EPOLLIN | EPOLLRDHUP |EPOLLET;
        // primaryChannel.epEvent.data.ptr = primaryChannel;

        epoll_ctl(epFd, EPOLL_CTL_ADD, fd, &primaryChannel->epEvent);

        logFunc("Epoller::ctr", "registering primary channel.");
        registeredChannels[fd] = primaryChannel;
        registeredChannels[fd]->epEvent.data.fd = fd;

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

        logFunc("Epoller::startPolling", "read pipe fd : " + std::to_string(exitFdTrx[0]) );
        logFunc("Epoller::startPolling", "write pipe fd : " + std::to_string(exitFdTrx[1]) );

        Channel pipedChannel {0};
        pipedChannel.fd = exitFdTrx[0];
        pipedChannel.is_primary = false;
        pipedChannel.is_exit_notifier = true;
        pipedChannel.epEvent = {0};
        pipedChannel.epEvent.events = EPOLLIN |EPOLLET;
        pipedChannel.epEvent.data.fd = pipedChannel.fd;

        registeredChannels[pipedChannel.fd] = std::make_shared<Channel>(pipedChannel);

        logFunc("Epoller::startPolling", "registering exiter pipe with kernel via epoll" + std::to_string(pipedChannel.fd));
        epoll_ctl(epFd, EPOLL_CTL_ADD, exitFdTrx[0], &pipedChannel.epEvent);

        int eventSize = 20;
        epoll_event events[eventSize];
        int waitDurationMilli = 500;
        int readDescriptors;
        bool exit = false;

        logFunc("Epoller::startPolling", "epoll loop will began.");
        int logCounter = 0;

        for(;;)
        {
            readDescriptors = epoll_wait(epFd, events, eventSize, waitDurationMilli);

            if(logCounter++ > 30) {
                logFunc("Epoller::startPolling", "epoll_wait timeup checking readDescriptors... " +
                                                 std::to_string(readDescriptors));
                logCounter = 0;
            }

            if (readDescriptors == -1) {
                logFunc("Epoller::startPolling", "epoll_wait failed.  exiting fd is -1");
                exit = true;
            }

            for (int i = 0; i < readDescriptors; i++)
            {
                logFunc("Epoller::startPolling", "iterating over each descriptor for which event was raised.");
                // if some error ( // error case)
                if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN)))
                {

                    logFunc("Epoller::startPolling", "An error occured ....");
                    auto channel = findChannel(events[i].data.fd);

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

                logFunc("Epoller::startPolling", "accessing the event");
                auto currentEvent = events[i];
                logFunc("Epoller::startPolling", "trying to get channel info for fd: " + std::to_string(currentEvent.data.fd));
                auto channelptr = findChannel(currentEvent.data.fd);

                if(!channelptr) // fd is totally new and probably a new request
                {
                    channelptr = findChannel(serverFd);
                }

                Channel channel = *channelptr;
                logFunc("Epoller::startPolling", "casted out channel");

                if (  channel.is_primary)
                {
                    logFunc("Epoller::startPolling", "new connection is coming request thru primary fd.");
                    //Channel primaryChannel = *reinterpret_cast<Channel*>(events[i].data.ptr);
                    newRequestHandler(channel);
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
                else if ( channel.is_exit_notifier)
                {
                    logFunc("Epoller::startPolling", "epoll_wait failed due to exit_notifier.  exiting");
                    // if exitFlag/Message
                    // Close method was invoked and called pipe write fd
                    // destructor should auto clean up
                    exit = true;
                    break;
                }
                else { // if existing client
                    logFunc("Epoller::startPolling", "existing client connected.");
                   // Channel clientChannel = *reinterpret_cast<Channel*>(events[i].data.ptr);
                    existingClientRequestHandler(channel);
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

            if(exit == true )
            {
                logFunc("Epoller::startPolling", "epoll_wait failed or exit was requested.  exiting");
                break;
            }
        }

        logFunc("Epoller::startPolling", "end");
    }
}

