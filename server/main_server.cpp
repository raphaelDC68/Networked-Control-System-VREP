#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>

#include "config.h"
#include "msg.h"
#include "SockManager.h"

extern "C" {
    #include "extApi.h"
}

using namespace std;

pthread_t rx_thread;
pthread_t tx_thread;

SockManager *sock;

int vrep;
int vrep_handles[CHANNELS];
pthread_mutex_t vrep_mutex;

void *rx(void *arg);
void *tx(void *arg);

void vrep_open();
void vrep_close();
void vrep_finish();
void vrep_get_handles();
void vrep_set(int channel, double value);
double vrep_get(int channel);
double vrep_time();

int main() 
{
	
	// Set up the socket addresses for the server	
	struct sockaddr_in tx_addr;
	struct sockaddr_in rx_addr;

    memset(&tx_addr, 0, sizeof(tx_addr));
    tx_addr.sin_family = AF_INET; // IPv4
    tx_addr.sin_addr.s_addr = inet_addr(SERVER_TX_ADDR);
    tx_addr.sin_port = htons(SERVER_TX_PORT);

    memset(&rx_addr, 0, sizeof(rx_addr));
    rx_addr.sin_family = AF_INET; // IPv4
    rx_addr.sin_addr.s_addr = INADDR_ANY;
    rx_addr.sin_port = htons(SERVER_RX_PORT);

	// Create a new socket manager
	sock = new SockManager(tx_addr, rx_addr);

	// Open the V-REP connection
	vrep_open();
	
	// Create two threads, one for receiving messages and one for transmitting messages
	pthread_create(&rx_thread, NULL, rx, NULL);
	pthread_create(&tx_thread, NULL, tx, NULL);

	// Join the threads before closing the V-REP connection and terminating the program
	pthread_join(rx_thread, NULL);
	pthread_join(tx_thread, NULL);
		
	// Close the V-REP connection
	vrep_close();

return 0;
}

void *rx(void *arg)
{
    size_t i; 
    ssize_t n; 
    socklen_t len; 
    msg_t msg; 
    int current, last; 
    float period; 

    while(true) { 
        msg = sock->recv(); 

        if(vrep_time() > msg.time) { // if message is late
            printf("late packet\n"); // print a warning message
        }

        while(vrep_time() < msg.time) { // wait until it's time to process the message
            usleep(1000); // sleep for 1 millisecond
        }
        
        current = vrep_time(); // get current time
        period = (current - last) / 1000.0; // calculate time period
        last = current; // update last time

        for(i = 0; i < CHANNELS; i++) { 
            vrep_set(i, msg.channels[i]);
        }

        print_msg(msg); 
        printf("period: %6.4f\n", period); 
    }

    return NULL;
}

void *tx(void *arg)
{
    size_t i, j; 
    msg_t msg; 

    for(i = 0; true; i++) { 
        msg.id = i; 

        for(j = 0; j < CHANNELS; j++) { 
            msg.channels[j] = vrep_get(j); 
        }

        msg.time = vrep_time();
        gettimeofday(&msg.sys_time, NULL); // get system time

        sock->send(&msg); 

        usleep(PERIOD * 1000); 
    }

    return NULL;
}

void vrep_open()
{   
    // Initialize mutex
    pthread_mutex_init(&vrep_mutex, NULL);

    // Start V-REP remote API
    vrep = simxStart((simxChar*) VREP_ADDR, VREP_PORT, true, true,
        VREP_TIMEOUT, VREP_CYCLE);

    // Check if connection was successful
    if(vrep == -1) {
        printf("Error: vrep_init\n");
    }

    // Enable synchronous mode (blocking)
    simxSynchronous(vrep, true);
    // Start simulation
    simxStartSimulation(vrep, simx_opmode_oneshot);

    // Get handles of the joints
    vrep_get_handles();

    // Disable position control of joints
    for(int i = 0; i < CHANNELS; i++) {
        simxSetObjectIntParameter(vrep, vrep_handles[i], 2001, 0,
            simx_opmode_oneshot);
    }
}

void vrep_close()
{   
    // Stop simulation and disconnect from V-REP
    simxStopSimulation(vrep, simx_opmode_oneshot);
    simxFinish(vrep);

    // Destroy mutex
    pthread_mutex_destroy(&vrep_mutex);
}

void vrep_get_handles()
{
    // Lock mutex
    pthread_mutex_lock(&vrep_mutex);

    // Get handles of the joints
    simxChar object_name[100];
    simxInt handle;
    simxInt res;
    int i = 0;

    for (i = 0; i < CHANNELS; i++) {
        // Construct joint name
        snprintf(object_name, 100, "joint%d", i+1);
        
        // Get handle of joint
        res = simxGetObjectHandle(vrep, object_name, &handle,
            simx_opmode_oneshot_wait);
        
        // Check if handle was obtained successfully
        if (res == simx_return_ok) {
            vrep_handles[i] = handle;
        } else {
            printf("Error: get_vrep_handles %s\n", object_name);
            exit(EXIT_FAILURE);
        }
    }

    // Unlock mutex
    pthread_mutex_unlock(&vrep_mutex);
}

/*
 * TODO: Pause the communication thread to update all values at the same time.
 */
void vrep_set(int channel, double value)
{
    pthread_mutex_lock(&vrep_mutex);

    //simxPauseCommunication(vrep, 1);
    
    simxSetJointTargetVelocity(vrep, vrep_handles[channel], (simxFloat) value,
        simx_opmode_oneshot);
  
    //simxPauseCommunication(vrep, 0);

    pthread_mutex_unlock(&vrep_mutex);
}

double vrep_get(int channel)
{
    simxFloat value;

    pthread_mutex_lock(&vrep_mutex);

    simxGetJointPosition(vrep, vrep_handles[channel], &value,
        simx_opmode_oneshot);

    pthread_mutex_unlock(&vrep_mutex);

    return (double) value;
}

double vrep_time()
{
    simxInt value;

    pthread_mutex_lock(&vrep_mutex);

    value = simxGetLastCmdTime(vrep);

    pthread_mutex_unlock(&vrep_mutex);

    return (int) value;
}