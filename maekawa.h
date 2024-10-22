#ifndef MAEKAWA_H
#define MAEKAWA_H

#define OK_MSG "OK"
#define REQUEST_MSG "REQUEST"
#define RELEASE_MSG "RELEASE"

#include <netinet/in.h>
#include "vector_clock.h"

// Define the Distributed Mutex structure
typedef struct {
    int process_id;              // This process ID
    VectorClock vc;              // Vector clock for this process
    int *voting_group;           // Voting group members
    int voting_group_size;       // Size of the voting group
    struct sockaddr_in *hosts;   // Array of hosts for communication
    int num_hosts;               // Number of hosts in the system
    int socket_fd;               // Socket file descriptor

    int votes_received;          // Number of votes received (OKs)
    int has_lock;                // Whether this process has the lock
    int vote_given_to;           // Process ID to which we gave the vote (-1 if none)
} CDistributedMutex;

// Function declarations
void GlobalInitialize(CDistributedMutex *mutex, int thisHost, struct sockaddr_in *hosts, int num_hosts);
void MInitialize(CDistributedMutex *mutex, int num_processes);
void MLockMutex(CDistributedMutex *mutex);
void MReleaseMutex(CDistributedMutex *mutex);
void MCleanup(CDistributedMutex *mutex);
void QuitAndCleanup(CDistributedMutex *mutex);
void handle_message(CDistributedMutex *mutex);
void handle_lock_request(CDistributedMutex *mutex, int sender);

#endif // MAEKAWA_H
