#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline)) 
long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    long total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        long sum = 0;
        int i;
        
        /* Complex inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Load operations with potential cache effects */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with higher latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency with distance-1 */
            if (i > 0) {
                /* Multiple uses of the same value to create distance1_uses */
                int prev_val = a[i-1];
                int temp = prev_val & 0xFF;  /* Bitwise operation */
                a[i] = (a[i] + temp) ^ product;  /* Complex update */
                
                /* Additional operation using the same product */
                b[i] = b[i] | (product & 0xFFFF);
            } else {
                /* Boundary case */
                a[i] = a[i] ^ product;
                b[i] = b[i] & ~product;
            }
            
            /* More operations to create scheduling pressure */
            int diff = a_val - b_val;
            sum = sum - (diff / 2);  /* Division creates additional latency */
            
            /* Conditional to prevent over-optimization */
            if (product & 1) {
                sum = sum ^ 0x1;  /* Bitwise operation */
            }
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Volatile conditional to prevent optimization */
        if (rand() % 100 > 50) {
            outer_counter--;  /* Occasionally repeat iteration */
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
    
    /* Call the complex loop function */
    long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    /* Additional print to use arrays and prevent optimization */
    printf("Sample values: a[0]=%d, b[0]=%d\n", a[0], b[0]);
    
    return 0;
}
