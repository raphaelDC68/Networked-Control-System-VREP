#pragma once

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>

#include "msg.h"

class SockManager
{
public:
    SockManager(struct sockaddr_in tx, struct sockaddr_in rx);
    ~SockManager();
    
    void send(msg_t *msg);
    msg_t recv();
    
    void routine();
    static void *helper(void *args);

private:
    int sockfd;
    msg_t last_received;
    struct sockaddr_in tx_addr;
    struct sockaddr_in rx_addr;
    pthread_t thread;
    bool running;
    pthread_mutex_t mutex;
    sem_t pending;
};