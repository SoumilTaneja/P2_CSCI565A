#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "maekawa.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <process_id>\n", argv[0]);
        return 1;
    }

    int process_id = atoi(argv[1]);
    int num_processes = 9;  // Assuming a 3x3 grid for non-disjoint quorums

    // Define host addresses (example with 9 processes)
    struct sockaddr_in hosts[num_processes];
    for (int i = 0; i < num_processes; i++) {
        hosts[i].sin_family = AF_INET;
        hosts[i].sin_port = htons(8080 + i); // Different port for each process
        inet_pton(AF_INET, "127.0.0.1", &hosts[i].sin_addr); // Localhost for all processes
    }

    // Create and initialize the mutex
    CDistributedMutex mutex;
    GlobalInitialize(&mutex, process_id, hosts, num_processes);  // Use process_id from arguments

    // Initialize Maekawa’s algorithm with the number of processes
    MInitialize(&mutex, num_processes);  // Only need to pass num_processes now

    // Request lock, enter critical section, then release lock
    MLockMutex(&mutex);
    MReleaseMutex(&mutex);

    // Cleanup
    MCleanup(&mutex);
    QuitAndCleanup(&mutex);

    return 0;
}
