#ifndef VECTOR_CLOCK_H
#define VECTOR_CLOCK_H

#define MAX_PROCESSES 100

// Vector clock structure
typedef struct {
    int clock[MAX_PROCESSES];  // The vector clock array
    int size;                  // Number of processes in the system
} VectorClock;

// Initialize vector clock with zeros
void initialize_vector_clock(VectorClock *vc, int num_processes);

// Increment the clock for the current process
void increment_vector_clock(VectorClock *vc, int process_id);

// Update the vector clock upon receiving a message
void update_vector_clock(VectorClock *vc, VectorClock *received_vc);

// Print the vector clock for debugging and output
void print_vector_clock(VectorClock *vc, int process_id);

#endif // VECTOR_CLOCK_H
