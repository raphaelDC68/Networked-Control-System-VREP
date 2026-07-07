# Networked Control System (NCS) & Latency Compensator

## Project Context
In modern industrial and remote-operated applications, robotic systems are often controlled over networks. However, network latency (transmission delay) inherently destabilizes closed-loop control systems. 

This project implements a complete **Networked Control System (NCS)** developed in C++. It features a hardware simulator (V-REP), a network degradation emulator, and a smart control client capable of dynamically adapting its control law to maintain robotic stability despite network fluctuations.

## System Architecture
The project is divided into three C++ nodes communicating via a custom UDP protocol, supported by an initial mathematical modeling phase in Python:

1. **Robotic Server (`/server`):**
   * Interfaces synchronously with the V-REP (CoppeliaSim) Remote API.
   * Utilizes POSIX threads (`pthread`) to broadcast the robot's joint states and receive incoming velocity commands simultaneously.

2. **Network Latency Emulator (`/retard`):**
   * A multithreaded network proxy acting as an artificial bottleneck.
   * Uses `std::queue` and POSIX Mutexes to intercept, store, and release UDP packets with strict delays, effectively simulating a degraded network environment.

3. **Control Client (`/client`):**
   * The remote "brain" of the system. Computes the target error and generates the appropriate command.
   * **Dynamic Gain Adaptation:** Calculates the Round-Trip Time (RTT) of every packet in real-time. If the latency crosses specific mathematical stability thresholds, the client dynamically dampens its gain matrix ($K$) to prevent the system from oscillating.
   * Uses the `Eigen3` linear algebra library for efficient matrix computations.

4. **Modeling & Stability (`/notebooks`):**
   * Mathematical modeling of the closed-loop transfer function and root locus analysis (Z-Transform) using `SymPy` and `SciPy`.
   * Theoretical determination of critical gains prior to the C++ bare-metal implementation.

## Tech Stack
* **Languages:** C++, Python (Jupyter)
* **C++ Libraries:** POSIX Threads (`pthread`), UDP Sockets (`<sys/socket.h>`), `Eigen3`
* **Simulation Environment:** V-REP PRO EDU (CoppeliaSim) Remote API
* **Data Analysis:** NumPy, Matplotlib, SymPy, SciPy

## Installation & Build

### Prerequisites
* GCC Compiler (`g++`)
* Eigen3 Library (`sudo apt-get install libeigen-dev`)
* V-REP PRO EDU v3.5.0 (or compatible)

### Compilation
Each module has its own `Makefile` and relies on the shared `/common` library.
To compile a specific module (e.g., the client):
```bash
cd client
make
