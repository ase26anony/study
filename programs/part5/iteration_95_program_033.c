/* modulo-sched-trigger.c
 * Designed to trigger uncovered lines in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-all -fdump-rtl-sched2 -march=native -o modulo_test modulo-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define INNER_ITERATIONS 1000
#define OUTER_ITERATIONS 5

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
unsigned long long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent optimization of outer loop */
    unsigned long long total_sum = 0;
    int i, j;
    
    /* Outer loop with volatile control */
    for (j = 0; j < OUTER_ITERATIONS; j++) {
        outer_counter = j;  /* Volatile write to prevent unrolling */
        
        /* Create control flow variability */
        if (rand() % 2) {
            /* Inner loop with multiple dependencies */
            unsigned long long sum = total_sum;
            int prev_val = a[0];  /* Loop-carried dependency seed */
            
            /* Complex inner loop designed to trigger modulo scheduling */
            for (i = 0; i < size; i++) {
                /* Multiple loads with potential cache effects */
                int a_val = a[i];
                int b_val = b[i];
                
                /* Operations with different latencies */
                int product = a_val * b_val;      /* Multi-cycle multiply */
                int shifted = product >> 3;       /* Fast operation */
                int masked = shifted & 0xFF;      /* Bitwise operation */
                
                /* Loop-carried dependency on sum */
                sum = sum + masked;               /* Distance-1 dependency */
                
                /* Another loop-carried dependency on array */
                int new_val;
                if (i > 0) {
                    /* Distance-1 dependency through array */
                    new_val = prev_val + b_val;   /* True loop-carried dep */
                } else {
                    new_val = a_val;
                }
                
                /* Multiple uses of computed value (creates distance1_uses) */
                int temp1 = new_val * 3;
                int temp2 = new_val + 7;
                int combined = temp1 ^ temp2;     /* Bitwise operation */
                
                /* Store with potential aliasing */
                a[i] = combined;
                prev_val = new_val;               /* Update for next iteration */
                
                /* Additional arithmetic to increase register pressure */
                total_sum += (sum & 1);           /* Use sum in another way */
            }
            
            total_sum = sum;
        } else {
            /* Alternative path to create control flow */
            for (i = 0; i < size; i++) {
                a[i] = a[i] + b[i] * 2;
            }
        }
        
        /* Volatile read to prevent loop optimization */
        asm volatile("" : : "r"(outer_counter));
    }
    
    return total_sum;
}

int main() {
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    int i;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    unsigned long long result = modulo_scheduled_loop(a, b, INNER_ITERATIONS);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %llu\n", result);
    
    /* Also print some array values to ensure stores aren't optimized away */
    printf("Sample values: a[0]=%d, a[100]=%d, b[0]=%d\n", 
           a[0], a[100], b[0]);
    
    return 0;
}
