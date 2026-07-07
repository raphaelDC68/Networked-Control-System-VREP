#include <math.h>           
#include <stdio.h>         
#include <string.h>       
#include <stdlib.h>        
#include <unistd.h>     

#include <fcntl.h>        
#include <sys/types.h>    
#include <sys/socket.h>   
#include <netinet/in.h>     

#include <Eigen/Dense>      // Linear algebra library
#include <Eigen/Core>       // Core linear algebra library

#include "config.h"         
#include "msg.h"           
#include "SockManager.h"    

using namespace Eigen;      // Using Eigen namespace
using namespace std;        // Using standard namespace

int main() {
	
    // Declare variables
    msg_t msg;                  
    struct sockaddr_in tx_addr; 
    struct sockaddr_in rx_addr; 
    struct timeval start;       
    struct timeval end;         
    size_t i, j;                
    int last_time;              
    unsigned long long delta;   
    SockManager *sock;          
    float k;                    

    // Declare arrays
    ArrayXd in(CHANNELS);       
    ArrayXd fb(CHANNELS);      
    ArrayXd err(CHANNELS);      
    ArrayXd out(CHANNELS);      
    ArrayXd K(CHANNELS);        

    // Initialize socket addresses
    memset(&tx_addr, 0, sizeof(tx_addr));  
    tx_addr.sin_family = AF_INET;           // IPv4
    tx_addr.sin_addr.s_addr = inet_addr(CLIENT_TX_ADDR); 
    tx_addr.sin_port = htons(CLIENT_TX_PORT);            

    memset(&rx_addr, 0, sizeof(rx_addr));  
    rx_addr.sin_family = AF_INET;           // IPv4
    rx_addr.sin_addr.s_addr = INADDR_ANY;   
    rx_addr.sin_port = htons(CLIENT_RX_PORT);           

    // Initialize socket manager
    sock = new SockManager(tx_addr, rx_addr);


    for(j = 0; j < CHANNELS; j++) {
        K[j] = 0; 
    }


    while(true) {   

        msg = sock->recv();     

 
        print_msg(msg);

        // Calculate time delta
        start = msg.sys_time;
        gettimeofday(&end, NULL);
        delta = (end.tv_sec - start.tv_sec) * 1000 +
            (end.tv_usec - start.tv_usec) / 1000;

        // Set gain factor based on time delta
        if(delta < 120) {
            K[1] = 2.01;
        } else if(delta < 230) {
            K[1] = 1.31;
        } else if (delta < 350) {
            K[1] = 1.21;
        } else {
            K[1] =  1.21;
            printf("non controlable \n"); 
        }

        // Calculate input and feedback values for each channel
        for(j = 0; j < CHANNELS; j++) {
            in[j] = 0.5*(msg.time % 40000 > msg.time % 20000) - 0.25;
            fb[j] = msg.channels[j];
        }
        
        // Calculate error values for each channel
        err = in - fb;
        
        // Calculate output values for each channel
        out = err * K;
        
        // Set message channels to output values
        for(j = 0; j < CHANNELS; j++) {
            msg.channels[j] = out[j];
        }
        
        // Print relevant values
        printf("time: %6.4lf\n", msg.time / 1000.0);
        printf("retard: %6.4lf\n", delta / 1000.0);
        printf("in[1]: %6.4lf\n", in[1]);
        printf("fb[1]: %6.4lf\n", fb[1]);
        printf("err[1]: %6.4lf\n", err[1]);
        printf("out[1]: %6.4lf\n", out[1]);
        
        // Update message time and send message
        msg.time += PERIOD;
        sock->send(&msg);
	}

    return 0;
}