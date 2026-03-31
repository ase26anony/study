#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
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
            /* Load operations - create RAW dependencies */
            int a_val = a[i];
            int b_val = b[i];
            
            /* High-latency operation (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency on sum (distance-1) */
            sum = sum + product;
            
            /* Another loop-carried dependency on array a (distance-1) */
            if (i > 0) {
                /* Multiple uses of the same value to create distance1_uses */
                int prev_val = a[i-1];
                int temp = prev_val + b_val;      /* First use */
                a[i] = (temp & 0xFF) | product;   /* Second use with bitwise ops */
                
                /* Create more scheduling complexity */
                int mixed = (temp ^ product) + (prev_val & b_val);
                sum += mixed % 256;  /* Additional accumulation */
            } else {
                /* Boundary case */
                a[i] = (a_val + b_val) & 0xFF;
            }
            
            /* Additional operations with different latencies */
            int extra_op = (a_val ^ b_val) * 3;  /* Mix of bitwise and multiply */
            sum += extra_op;
            
            /* Create distance-0 dependencies within same iteration */
            int chain1 = product + extra_op;
            int chain2 = chain1 * 2;
            sum += chain2;
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional branch based on random to prevent optimization */
        if (rand() % 100 > 50) {
            total_sum += 1;  /* Small perturbation */
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call the loop function */
    long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    /* Verify some computation */
    printf("First few a[] values after computation: ");
    for (i = 0; i < 5; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
    
    return 0;
}
