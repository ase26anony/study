#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline))
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = OUTER_ITERATIONS;  /* Prevent unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter-- > 0) {
        int sum = 0;
        volatile int use_alternate = (rand() % 2);  /* Control flow variability */
        
        /* Critical inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            if (i > 0) {
                /* Multiple uses of the same computed value */
                int prev_val = a[i-1];
                int temp = prev_val & 0xFF;  /* Bitwise operation */
                a[i] = (a[i] + temp) | (prev_val ^ 0x55);  /* Mixed operations */
            }
            
            /* Another distance-0 dependency within same iteration */
            int shifted = product >> 3;
            b[i] = (b[i] - shifted) & 0x7FFF;
            
            /* Additional arithmetic to create more scheduling pressure */
            if (use_alternate) {
                sum = sum ^ (a_val & b_val);  /* Bitwise operation */
            }
        }
        
        total_sum += sum;
        
        /* Small computation between outer loop iterations */
        for (i = 0; i < 10; i++) {
            total_sum = (total_sum * 31) + 17;  /* Prevent optimization */
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i, result;
    
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional print to ensure arrays are used */
    printf("Sample values: a[0]=%d, b[0]=%d\n", a[0], b[0]);
    
    return 0;
}
