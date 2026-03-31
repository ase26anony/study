#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITER 5

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline)) 
unsigned long long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    unsigned long long total_sum = 0;
    
    /* Outer loop with volatile control */
    for (outer_counter = 0; outer_counter < OUTER_ITER; outer_counter++) {
        unsigned long long sum = 0;
        int prev_val = 0;
        
        /* Create conditional branch in outer loop */
        if (rand() % 2) {
            prev_val = 1;
        }
        
        /* Inner loop with complex dependencies */
        for (int i = 0; i < size; i++) {
            /* Load operations with potential cache effects */
            volatile int a_val = a[i];  /* Prevent load combining */
            volatile int b_val = b[i];  /* Prevent load combining */
            
            /* Multiple operations with different latencies */
            int product = a_val * b_val;      /* Higher latency multiply */
            int shifted = product >> 3;       /* Shift operation */
            int masked = shifted & 0xFF;      /* Bitwise operation */
            
            /* Loop-carried dependency on sum */
            sum = sum + masked;               /* Distance-1 dependency */
            
            /* Another loop-carried dependency on array */
            int new_val;
            if (i > 0) {
                /* Distance-1 dependency: a[i] depends on a[i-1] */
                new_val = a[i-1] + b_val + prev_val;
            } else {
                new_val = b_val + prev_val;
            }
            
            /* Multiple uses of the same value (creates distance1_uses) */
            int temp = new_val * 2;           /* Use 1: multiply */
            temp = temp ^ 0xAAAA;             /* Use 2: bitwise XOR */
            a[i] = temp & 0xFFFF;             /* Use 3: bitwise AND and store */
            
            /* Additional arithmetic to create more scheduling pressure */
            prev_val = (prev_val * 3 + i) & 0xFF;
            
            /* Another accumulation with different operation */
            sum = sum + (product & 0xF);
        }
        
        total_sum += sum;
        
        /* Volatile operation to prevent loop optimizations */
        asm volatile("" : : "r"(total_sum) : "memory");
    }
    
    return total_sum;
}

int main() {
    /* Initialize with different seed each run */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
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
    
    /* Additional print to ensure value is used */
    volatile unsigned long long vol_result = result;
    printf("Volatile result: %llu\n", vol_result);
    
    free(a);
    free(b);
    
    return 0;
}
