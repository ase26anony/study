/* modulo-sched-trigger.c
 * Designed to trigger GCC's modulo scheduler debugging output
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-all -fdump-rtl-sched2 -march=native -o modulo_test modulo-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    long total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        long sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Critical inner loop with cross-iteration dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Integer multiply (higher latency on x86) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another operation using the product (creates multiple uses) */
            int shifted = product >> 3;
            
            /* Loop-carried dependency through array: a[i] depends on a[i-1] */
            int new_a_val;
            if (i > 0) {
                /* distance-1 dependency: uses value from previous iteration */
                new_a_val = prev_val + b_val + shifted;
            } else {
                new_a_val = a_val + b_val;
            }
            
            /* Bitwise operations (different latency characteristics) */
            new_a_val = (new_a_val & 0xFFF) | (shifted << 12);
            
            /* Store back with volatile to prevent optimization */
            *(volatile int *)&a[i] = new_a_val;
            prev_val = new_a_val;
            
            /* Additional arithmetic to create more scheduling opportunities */
            int temp = (product & 0xF) * (b_val & 0xF);
            sum = sum - (temp / 2);  /* Another loop-carried use of sum */
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional branch based on random to create control variability */
        if (rand() % 2) {
            /* Extra operation that might affect scheduling */
            total_sum ^= 0xABCD;
        }
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
    
    /* Call the function with the computationally intensive loop */
    long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    /* Also print some array values to ensure they're used */
    printf("Sample a[0]=%d, a[SIZE-1]=%d\n", a[0], a[SIZE-1]);
    
    return 0;
}
