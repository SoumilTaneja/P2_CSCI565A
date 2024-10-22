#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>  // For nanosleep
#include <netdb.h>  // For hostname retrieval
#include <math.h>   // For sqrt() in quorum formation
#include "maekawa.h"

// Function to introduce a random delay (between 0 and 1 second)
void random_delay() {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = rand() % 1000000000L;  // Random delay between 0 and 1 second
    nanosleep(&ts, NULL);
}

// Function to get the hostname
void get_hostname(char* hostname, size_t size) {
    if (gethostname(hostname, size) != 0) {
        perror("gethostname failed");
        strcpy(hostname, "unknown");
    }
}

// Function to print the vector clock with the required format
void print_vector_clock_output(CDistributedMutex *mutex) {
    char hostname[256];
    get_hostname(hostname, sizeof(hostname));
    
    // Print in the required format: hostname:(vector clock entries)
    printf("%s:(", hostname);
    for (int i = 0; i < mutex->vc.size; i++) {
        printf("%d", mutex->vc.clock[i]);
        if (i < mutex->vc.size - 1) {
            printf(",");
        }
    }
    printf(")\n");
}

// Global initialization of the distributed mutex
void GlobalInitialize(CDistributedMutex *mutex, int thisHost, struct sockaddr_in *hosts, int num_hosts) {
    mutex->process_id = thisHost;
    mutex->hosts = hosts;
    mutex->num_hosts = num_hosts;
    mutex->votes_received = 0;
    mutex->has_lock = 0;
    mutex->vote_given_to = -1; // No vote given initially

    // Initialize the vector clock
    initialize_vector_clock(&mutex->vc, num_hosts);

    // Create a socket for communication
    mutex->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (mutex->socket_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the host address
    if (bind(mutex->socket_fd, (const struct sockaddr *)&hosts[thisHost], sizeof(hosts[thisHost])) < 0) {
        perror("Socket bind failed");
        close(mutex->socket_fd);
        exit(EXIT_FAILURE);
    }

    // Set socket to non-blocking mode to handle incoming messages
    fcntl(mutex->socket_fd, F_SETFL, O_NONBLOCK);

    printf("Process %d initialized and bound to socket.\n", thisHost);
}

// Initialize Maekawa’s algorithm with pairwise non-disjoint quorums
void MInitialize(CDistributedMutex *mutex, int num_processes) {
    int quorum_size = (int)sqrt(num_processes);  // Define quorum size based on grid
    mutex->voting_group_size = 2 * quorum_size - 1;  // Includes row and column minus overlap
    mutex->voting_group = (int *)malloc(mutex->voting_group_size * sizeof(int));

    int row = mutex->process_id / quorum_size;  // Row in the grid
    int col = mutex->process_id % quorum_size;  // Column in the grid

    // Add all processes in the same row to the quorum
    int index = 0;
    for (int i = 0; i < quorum_size; i++) {
        int row_process = row * quorum_size + i;
        if (row_process != mutex->process_id) {
            mutex->voting_group[index++] = row_process;
        }
    }

    // Add all processes in the same column to the quorum
    for (int i = 0; i < quorum_size; i++) {
        int col_process = i * quorum_size + col;
        if (col_process != mutex->process_id) {
            mutex->voting_group[index++] = col_process;
        }
    }

    // Print the quorum for the current process
    printf("Process %d initialized Maekawa's voting group: [", mutex->process_id);
    for (int i = 0; i < mutex->voting_group_size; i++) {
        printf("%d", mutex->voting_group[i]);
        if (i < mutex->voting_group_size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Request a lock for the critical section and wait for OKs
void MLockMutex(CDistributedMutex *mutex) {
    // Increment the vector clock before sending a request
    increment_vector_clock(&mutex->vc, mutex->process_id);
    print_vector_clock_output(mutex);  // Print the updated vector clock

    // Multicast the lock request to the voting group
    for (int i = 0; i < mutex->voting_group_size; i++) {
        int target = mutex->voting_group[i];
        printf("Process %d sending lock request to process %d\n", mutex->process_id, target);

        // Random delay before sending the message to simulate network delays
        random_delay();

        // Send a REQUEST message to each process in the voting group
        sendto(mutex->socket_fd, REQUEST_MSG, strlen(REQUEST_MSG), 0,
               (struct sockaddr *)&mutex->hosts[target], sizeof(mutex->hosts[target]));
    }

    // Wait for OK messages from all processes in the voting group
    mutex->votes_received = 0;
    while (mutex->votes_received < mutex->voting_group_size) {
        handle_message(mutex);
    }

    mutex->has_lock = 1;
    printf("Process %d entering critical section.\n", mutex->process_id);
}

// Handle incoming messages (OK, REQUEST, and RELEASE)
void handle_message(CDistributedMutex *mutex) {
    char buffer[1024];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    // Use select to wait for incoming messages
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mutex->socket_fd, &readfds);

    struct timeval timeout = {1, 0}; // 1-second timeout

    int activity = select(mutex->socket_fd + 1, &readfds, NULL, NULL, &timeout);

    if (activity > 0 && FD_ISSET(mutex->socket_fd, &readfds)) {
        recvfrom(mutex->socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &addr_len);
        buffer[sizeof(buffer) - 1] = '\0'; // Null-terminate the message

        // Random delay to simulate message handling delays
        random_delay();

        if (strcmp(buffer, OK_MSG) == 0) {
            printf("Process %d received OK from a process\n", mutex->process_id);
            mutex->votes_received++;
        } else if (strcmp(buffer, REQUEST_MSG) == 0) {
            // Identify which process sent the request (extract sender info from src_addr)
            int sender = -1;
            for (int i = 0; i < mutex->num_hosts; i++) {
                if (memcmp(&mutex->hosts[i].sin_addr, &src_addr.sin_addr, sizeof(src_addr.sin_addr)) == 0) {
                    sender = i;
                    break;
                }
            }
            handle_lock_request(mutex, sender);
        } else if (strcmp(buffer, RELEASE_MSG) == 0) {
            printf("Process %d received RELEASE from a process\n", mutex->process_id);
            mutex->vote_given_to = -1; // Free the vote
        }
    }
}

// Handle a lock request (send OK if vote is free)
void handle_lock_request(CDistributedMutex *mutex, int sender) {
    printf("Process %d received lock request from process %d\n", mutex->process_id, sender);

    // Increment vector clock upon receiving a request
    increment_vector_clock(&mutex->vc, mutex->process_id);
    print_vector_clock_output(mutex);  // Print the updated vector clock

    // Check if we have given our vote to another process
    if (mutex->vote_given_to == -1) {
        printf("Process %d sending OK to process %d\n", mutex->process_id, sender);
        sendto(mutex->socket_fd, OK_MSG, strlen(OK_MSG), 0,
               (struct sockaddr *)&mutex->hosts[sender], sizeof(mutex->hosts[sender]));
        mutex->vote_given_to = sender; // Give the vote to this process
    } else {
        printf("Process %d already gave vote to process %d\n", mutex->process_id, mutex->vote_given_to);
    }
}

// Release the lock after exiting the critical section
void MReleaseMutex(CDistributedMutex *mutex) {
    mutex->has_lock = 0;

    // Multicast the release message to the voting group
    for (int i = 0; i < mutex->voting_group_size; i++) {
        int target = mutex->voting_group[i];
        printf("Process %d sending release message to process %d\n", mutex->process_id, target);

        // Send a release message to each process in the voting group
        sendto(mutex->socket_fd, RELEASE_MSG, strlen(RELEASE_MSG), 0,
               (struct sockaddr *)&mutex->hosts[target], sizeof(mutex->hosts[target]));
    }

    printf("Process %d exited critical section.\n", mutex->process_id);
}

// Cleanup Maekawa’s algorithm state
void MCleanup(CDistributedMutex *mutex) {
    free(mutex->voting_group);
    printf("Process %d cleaned up Maekawa's state.\n", mutex->process_id);
}

// Cleanup and close sockets
void QuitAndCleanup(CDistributedMutex *mutex) {
    close(mutex->socket_fd);
    printf("Process %d cleaned up and exited.\n", mutex->process_id);
}
