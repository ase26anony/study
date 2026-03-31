#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITER 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
unsigned long long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    unsigned long long total_sum = 0;
    
    /* Outer loop with volatile control */
    for (outer_counter = 0; outer_counter < OUTER_ITER; outer_counter++) {
        volatile int use_alt = rand() % 2;  /* Control flow variability */
        unsigned long long sum = 0;
        int prev_val = 0;
        
        /* Inner loop with complex dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with non-unit latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum accumulates across iterations */
            sum = sum + product;
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            int temp = 0;
            if (i > 0) {
                /* Create distance-1 use with multiple consumers */
                temp = a[i-1] + b_val;  /* distance1_uses scenario */
                
                /* Multiple uses of the same computed value */
                int temp2 = temp & 0xFF;  /* Bitwise operation */
                int temp3 = temp | 0x55;
                
                /* Chain of operations with different latencies */
                a_val = temp2 + temp3 - product;
            } else {
                a_val = b_val ^ 0xAA;  /* Different operation for first iteration */
            }
            
            /* Store with potential anti-dependencies */
            a[i] = a_val + (use_alt ? temp : 0);
            
            /* Additional arithmetic to increase register pressure */
            b[i] = (b_val * 3) + (sum & 0xFFFF);
        }
        
        /* Mix in control flow variability */
        if (use_alt) {
            total_sum += sum;
        } else {
            total_sum += sum * 2;
        }
        
        /* Prevent optimization of loop body */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return total_sum;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    unsigned long long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %llu\n", result);
    
    /* Verify computation */
    printf("Sample values: a[0]=%d, a[%d]=%d, b[0]=%d\n", 
           a[0], SIZE-1, a[SIZE-1], b[0]);
    
    free(a);
    free(b);
    
    return 0;
}
