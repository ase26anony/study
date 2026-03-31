#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
unsigned long long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    unsigned long long total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        unsigned long long sum = 0;
        int i;
        
        /* Inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads creating scheduling pressure */
            volatile int a_val = a[i];
            volatile int b_val = b[i];
            
            /* Operation with non-unit latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            if (i > 0) {
                /* Multiple uses of the same value to create distance1_uses */
                int prev_val = a[i-1];
                int temp = prev_val & 0xFF;  /* Bitwise operation */
                a[i] = (a[i] ^ temp) + prev_val;  /* Complex update with multiple ops */
            }
            
            /* Additional operations to create varied latencies */
            int extra_op = (product >> 3) | (b_val & 0x7F);
            b[i] = extra_op - (i % 256);
        }
        
        /* Mix in some conditional control flow */
        if (rand() % 2) {
            total_sum += sum;
        } else {
            total_sum += (sum >> 1);
        }
        
        outer_counter++;
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i;
    
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    unsigned long long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %llu\n", result);
    
    return 0;
}
