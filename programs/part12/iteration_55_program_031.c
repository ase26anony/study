#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

/* Simple LCG for generating pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function to create loop-carried dependencies */
int process_loop(volatile int* array, int n) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = 1;
    
    /* Create distance-1 dependencies: 
       prev from iteration i is used in iteration i+1 */
    for (int i = 0; i < n; i++) {
        /* Multiple arithmetic operations to create scheduling complexity */
        int val = array[i];
        
        /* Recurrence: sum depends on previous iteration's sum */
        sum = sum + val * (sum % 256);
        
        /* Distance-1 dependency chain */
        prev = curr;                /* prev_i = curr_{i-1} */
        curr = val + prev;          /* curr_i depends on prev_i (which is curr_{i-1}) */
        
        /* More arithmetic to increase instruction count */
        sum = sum ^ (curr << 3);
        sum = sum + (prev * 7);
        sum = (sum >> 1) | (sum << 31);  /* Rotate right */
    }
    
    return sum + prev + curr;
}

/* Outer loop to provide context */
void outer_loop(volatile int* data, int outer_iterations) {
    volatile int total = 0;
    
    for (int iter = 0; iter < outer_iterations; iter++) {
        /* Slightly modify data each outer iteration */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = (data[i] * 3 + 1) & 0xFF;
        }
        
        /* Call the target inner loop */
        total += process_loop(data, ARRAY_SIZE);
        
        /* Prevent over-optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    printf("Final total: %d\n", total);
}

int main(void) {
    /* Initialize array with volatile to prevent optimization */
    volatile int data[ARRAY_SIZE];
    
    /* Fill array with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = lcg_rand() % 256;
    }
    
    /* Execute the loop multiple times */
    outer_loop(data, 10);
    
    return 0;
}
