#pragma once

#define RETARD_ENABLED

#ifdef RETARD_ENABLED

#define SERVER_TX_ADDR "127.0.0.1"
#define SERVER_TX_PORT (8000)

#define SERVER_RX_ADDR "127.0.0.1"
#define SERVER_RX_PORT (8001)

#define CLIENT_TX_ADDR "127.0.0.1"
#define CLIENT_TX_PORT (8002)

#define CLIENT_RX_ADDR "127.0.0.1"
#define CLIENT_RX_PORT (8003)

#else

#define SERVER_TX_ADDR "127.0.0.1"
#define SERVER_TX_PORT (8000)

#define SERVER_RX_ADDR "127.0.0.1"
#define SERVER_RX_PORT (8001)

#define CLIENT_TX_ADDR "127.0.0.1"
#define CLIENT_TX_PORT (8001)

#define CLIENT_RX_ADDR "127.0.0.1"
#define CLIENT_RX_PORT (8000)

#endif

#define VREP_ADDR "127.0.0.1"
#define VREP_PORT (5555)
#define VREP_TIMEOUT (5000)
#define VREP_CYCLE (5)

#define CHANNELS (6)
#define PERIOD (10)