#include "SockManager.h"

SockManager::SockManager(struct sockaddr_in tx, struct sockaddr_in rx)
{
    int res;

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("Error: socket creation\n");
        exit(EXIT_FAILURE);
    }

    tx_addr = tx;
    rx_addr = rx;

    // Bind the socket with the server address
    res = bind(sockfd, (const struct sockaddr *) &rx_addr, sizeof(rx_addr));
    if (res < 0) {
        printf("Error: bind\n");
        perror("");
        exit(EXIT_FAILURE);
    }

    //fcntl(sockfd, F_SETFL, fcntl(sockfd,F_GETFL) | O_NONBLOCK); 

    memset(&last_received, 0, sizeof(msg_t));
    pthread_create(&thread, NULL, &helper, (void *) this);
    running = true;
    pthread_mutex_init(&mutex, NULL);
    sem_init(&pending, 0, 0);
}

SockManager::~SockManager()
{
    pthread_mutex_lock(&mutex);
    
    running = false;

    pthread_mutex_unlock(&mutex);

    pthread_join(thread, NULL);
}
    
void SockManager::send(msg_t *msg)
{
    int n;

    n = sendto(sockfd, (const char *) msg, sizeof(msg_t), 0,
        (const struct sockaddr *) &tx_addr, sizeof(tx_addr));

    if(n == -1) {
        perror("");
        printf("Error: sendto\n");
        exit(EXIT_FAILURE);
    }
}

msg_t SockManager::recv()
{
    msg_t res;

    sem_wait(&pending);

    pthread_mutex_lock(&mutex);

    res = last_received;

    pthread_mutex_unlock(&mutex);

    return res;
}

void SockManager::routine()
{
    int n;
    msg_t buffer;
    struct sockaddr src_addr;
    socklen_t addrlen;
    
    while(1) {
        n = recvfrom(sockfd, (char *) &buffer, sizeof(msg_t), 0,
            &src_addr, &addrlen);

        if(n == -1) {
            printf("Error: recvfrom\n");
            exit(EXIT_FAILURE);
        }

        pthread_mutex_lock(&mutex);

        last_received = buffer;
        sem_trywait(&pending); sem_post(&pending); // ensure binary

        if(!running) {
            break;
        }

        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_destroy(&mutex);
}

void *SockManager::helper(void *args)
{
    SockManager *obj = (SockManager *) args;

    obj->routine();

    return NULL;
}