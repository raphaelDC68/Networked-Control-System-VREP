#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <list>
#include <queue>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "config.h"
#include "msg.h"
#include "SockManager.h"

#define DELAY (50)
#define RELAYS_LEN (2)

typedef struct timeval timeval_t;

// Define a structure for storing a timeval timestamp and a message
typedef struct {
msg_t msg;
timeval_t time;
} element_t;

// Define a structure for storing relay arguments
typedef struct {
int rx_port;
int tx_port;
int delay;
} relay_arg_t;

// Define a structure for storing relay data
typedef struct {
SockManager *sock;
int delay;
bool running;
std::list<element_t> list;
pthread_mutex_t mutex;
} relay_data_t;

// Declare functions for relaying, receiving and transmitting
void *relay(void *ptr);
void *rx(void *ptr);
void *tx(void *ptr);

// Declare functions for working with time values
timeval_t now();
timeval_t add(const timeval_t &a, const timeval_t &b);


int main(int argc, char *argv[])
{
int delay;
relay_arg_t relay_args[RELAYS_LEN];
pthread_t relays[RELAYS_LEN];

    if(argc == 2) {
        delay = atoi(argv[1]);
    } else {
        delay = DELAY;
    }

    printf("delay: %d\n", delay);

    relay_args[0] = {SERVER_TX_PORT, CLIENT_RX_PORT, delay};
    relay_args[1] = {CLIENT_TX_PORT, SERVER_RX_PORT, delay};

    for(int i = 0; i < RELAYS_LEN; i++) {
        pthread_create(&relays[i], NULL, relay, &relay_args[i]);
    }
    
    for(int i = 0; i < RELAYS_LEN; i++) {
        pthread_join(relays[i], NULL);
    }

    return 0;
}

void *relay(void *ptr) {
    relay_arg_t *arg;
    relay_data_t *data;
    struct sockaddr_in rx_addr, tx_addr;
    pthread_t rx_thread, tx_thread;
    
    arg = (relay_arg_t *) ptr;
    data = new relay_data_t;

   // set up receive and transmit addresses
	memset(&rx_addr, 0, sizeof(rx_addr));
	rx_addr.sin_family = AF_INET; // IPv4
	rx_addr.sin_addr.s_addr = INADDR_ANY;
	rx_addr.sin_port = htons(arg->rx_port);

	memset(&tx_addr, 0, sizeof(tx_addr));
	tx_addr.sin_family = AF_INET; // IPv4
	tx_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	tx_addr.sin_port = htons(arg->tx_port);

	// initialize relay data with socket manager object, delay, running state, and list of elements
	data->sock = new SockManager(tx_addr, rx_addr);
	data->delay = arg->delay;
	data->running = true;
	data->list = std::list<element_t>();

	pthread_mutex_init(&data->mutex, NULL);

	// create receive and transmit threads	
	pthread_create(&rx_thread, NULL, rx, data);
	pthread_create(&tx_thread, NULL, tx, data);

	// wait for receive and transmit threads to finish
	pthread_join(rx_thread, NULL);
	pthread_join(tx_thread, NULL);

	pthread_mutex_destroy(&data->mutex);

	return 0;
}

void *tx(void *ptr)
{
    relay_data_t *data;
    std::queue<msg_t> queue;
    msg_t msg;
    timeval_t cur;

    data = (relay_data_t *) ptr;

    while(data->running) {
        cur = now();

        pthread_mutex_lock(&data->mutex);
        for (auto it = data->list.begin(); it != data->list.end();) {
            if (timercmp(&cur, &it->time, >=)) {
                queue.push(it->msg);
                it = data->list.erase(it);
            } else {
                ++it;
            }
        }
        pthread_mutex_unlock(&data->mutex);

        // go easy on the cpu
        if(queue.empty()) {
            usleep(1000);
        }

        while(!queue.empty()) {
            msg = queue.front();
            queue.pop();

            data->sock->send(&msg);
        }
    }

    return 0;
}

void *rx(void *ptr)
{
    relay_data_t *data;
    msg_t msg;
    timeval_t diff, time;

    data = (relay_data_t *) ptr;

	while(data->running) {
		msg = data->sock->recv();
		diff = {0, data->delay * 1000}; // delay converted to microseconds
		time = add(now(), diff); // add delay to current time
    
		// lock the mutex and add the message and timestamp to the list
		pthread_mutex_lock(&data->mutex);
		data->list.push_back({msg, time});
		pthread_mutex_unlock(&data->mutex);
	}
	return 0;
}


  

timeval_t now()
{
    timeval_t res;
    gettimeofday(&res, NULL);
    return res;
}

timeval_t add(const timeval_t &a, const timeval_t &b)
{
    timeval_t res;
    timeradd(&a, &b, &res);
    return res;
}


