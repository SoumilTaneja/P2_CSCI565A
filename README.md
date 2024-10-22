Name - Soumil Taneja
CWID - 10924723

## Maekawa's Mutual Exclusion Algorithm with Vector Clocks

## Overview

This project implements **Maekawa's Mutual Exclusion Algorithm** in C, along with **Vector Clocks** to ensure mutual exclusion and causal ordering in a distributed system. The implementation uses **UDP sockets** for communication between processes and simulates real-world distributed systems through random delays in message exchanges.

The program outputs vector timestamps every time a message is sent or received and maintains causal order by printing the vector clock in the correct format. Each process runs in a separate terminal or machine, requesting and releasing a distributed lock.

## Design Overview

### Maekawa's Algorithm
1. **Request Phase**:
   - A process sends `REQUEST` messages to its quorum (voting group) when it wants to enter the critical section.
   - A process can only vote (send an `OK`) to one process at a time. If it has already voted, it queues the request.
   
2. **Critical Section**:
   - Once a process receives all necessary `OK` votes from its quorum, it enters the critical section.
   
3. **Release Phase**:
   - After exiting the critical section, the process sends a `RELEASE` message to its quorum, releasing their votes.
   - Processes that had requests queued will vote (`OK`) for the next queued request.

### Vector Clocks
- Each process maintains a vector clock to track causality between events.
- The vector clock is incremented each time a message is sent or received, ensuring that the system maintains causal ordering.

### Assumptions
- There is a fixed number of processes in the system, known at startup.
- Each process communicates using UDP sockets, either on different machines or by simulating them on the same machine using different ports.
- Random network delays are introduced to simulate real-world conditions.

---

## How to Compile and Run the Program

### Step 1: Compilation

Compile the program using the provided **Makefile**.

```
make
```

This will generate the `maekawa` executable.

### Step 2: Running the Program

The program simulates multiple processes. Each process is identified by a unique process ID. Open multiple terminals (or run on different machines) and execute the following commands:

- **Terminal 1 (for Process 0)**:
    ```
    ./maekawa 0
    ```

- **Terminal 2 (for Process 1)**:
    ```
    ./maekawa 1
    ```

- **Terminal 3 (for Process 2)**:
    ```
    ./maekawa 2
    ```

Each process will:
- Initialize Maekawa's voting group.
- Send lock requests to other processes in its quorum.
- Receive votes and enter the critical section when all votes are obtained.
- Print vector timestamps on each message exchange.

### Output

1. **Vector Clock Output**: Each time a vector clock is updated, the process prints the updated vector clock in the format:
    ```
    <hostname>:(<vector clock entries>)
    ```

    Example output:
    ```
    MacBook-Air.local:(1,0,0)
    MacBook-Air.local:(2,0,0)
    ```

2. **Group Formation**: During initialization, the voting group for each process is printed:
    ```
    Process 0 initialized Maekawa's voting group: [1, 2]
    ```

### Step 3: Cleaning Up

To clean up the compiled executable and object files, use the `clean` target in the Makefile:

```
make clean
```

---

## Files Included

- `main.c`: The main entry point that initializes each process.
- `maekawa.c`: Implementation of Maekawa's Mutual Exclusion Algorithm.
- `vector_clock.c`: Implementation of vector clocks for causal ordering.
- `maekawa.h` & `vector_clock.h`: Header files containing function declarations.
- `Makefile`: Automates the compilation process.
- `README.md`: Provides instructions and an overview of the project.

---

## Project Design and Assumptions

1. **Voting Group**:
   - Each process has a predefined voting group, or quorum, of other processes it communicates with. The voting group is initialized when the process starts, and is essential for the mutual exclusion algorithm to work.

2. **Non-blocking Communication**:
   - The program uses non-blocking UDP sockets to ensure that each process can send and receive messages without waiting indefinitely. This allows the processes to simulate asynchronous communication, where messages can be delayed or dropped, just like in a real distributed system.

3. **Random Delays**:
   - To simulate real-world network conditions, the program introduces random delays when sending and receiving messages. These delays ensure that the algorithm is robust against timing issues.

4. **Concurrency Handling**:
   - The program handles concurrency by ensuring that each process can only grant its vote to one process at a time. If another request arrives while a vote has already been granted, the request is queued until the vote is released.

---

## Future Improvements

1. **Distributed Testing**: 
   - Although the program simulates a distributed environment using local ports, testing it on multiple machines would better replicate real-world distributed system conditions.

2. **Robustness Against Failures**: 
   - Currently, the system assumes all processes are reliable. Adding fault-tolerance mechanisms to handle crashes and recovery would improve the system's robustness.

---

Thank you!
