#include <stdio.h>
#include "vector_clock.h"

// Initialize the vector clock to all zeros
void initialize_vector_clock(VectorClock *vc, int num_processes) {
    vc->size = num_processes;
    for (int i = 0; i < num_processes; i++) {
        vc->clock[i] = 0;
    }
}

// Increment the vector clock for the current process
void increment_vector_clock(VectorClock *vc, int process_id) {
    vc->clock[process_id]++;
}

// Update the local vector clock using the received vector clock
void update_vector_clock(VectorClock *vc, VectorClock *received_vc) {
    for (int i = 0; i < vc->size; i++) {
        if (received_vc->clock[i] > vc->clock[i]) {
            vc->clock[i] = received_vc->clock[i];
        }
    }
}

// Print the vector clock in the format specified
void print_vector_clock(VectorClock *vc, int process_id) {
    printf("Process %d: (", process_id);
    for (int i = 0; i < vc->size; i++) {
        printf("%d", vc->clock[i]);
        if (i < vc->size - 1) {
            printf(",");
        }
    }
    printf(")\n");
}
