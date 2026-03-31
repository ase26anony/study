#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Simple LCG for pseudo-random values */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function to create loop-carried dependencies */
int process_array(volatile int* arr, int n, int init) {
    volatile int sum = init;
    volatile int prev = 0;
    volatile int curr = arr[0];
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset for each outer iteration */
        sum = init + outer;
        prev = 0;
        curr = arr[0];
        
        /* Target inner loop with loop-carried dependencies */
        for (int i = 0; i < n; i++) {
            /* Multiple arithmetic operations to create scheduling complexity */
            int val = arr[i];
            
            /* Recurrence 1: sum depends on previous iteration's sum */
            sum = sum + (val * (sum & 0xFF));
            
            /* Recurrence 2: distance-1 dependency chain */
            int temp = prev + (val >> 2);  /* Use prev from previous iteration */
            prev = curr;                    /* Set prev for next iteration */
            curr = temp * 3;                /* Complex operation for curr */
            
            /* More arithmetic to increase instruction count */
            sum = sum ^ (curr & 0x7F);
            sum = sum + (val * 2);
            
            /* Additional operations to create move opportunities */
            int extra = (sum << 1) | (val & 1);
            sum = sum - (extra / 4);
        }
    }
    
    return sum;
}

int main() {
    /* Create and initialize array with volatile to prevent optimization */
    volatile int array[SIZE];
    
    /* Fill array with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Volatile to ensure loop executes */
    volatile int iterations = SIZE;
    
    /* Execute the processing function multiple times */
    int total_result = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        total_result += process_array(array, iterations, repeat * 100);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    return 0;
}
