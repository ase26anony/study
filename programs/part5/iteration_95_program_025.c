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
        
        /* Inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Load operations with potential cache effects */
            volatile int a_val = a[i];
            volatile int b_val = b[i];
            
            /* Multiple operations with different latencies */
            int product = a_val * b_val;      /* Higher latency multiply */
            int shifted = product >> 3;       /* Shift operation */
            int masked = shifted & 0x1F;      /* Bitwise AND */
            
            /* Loop-carried dependency on sum */
            sum = sum + product + masked;     /* Multiple adds */
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            if (i > 0) {
                /* Complex update with multiple uses of previous value */
                int prev = a[i-1];
                int temp = prev + b_val;      /* Distance-1 use */
                a[i] = (temp * 3) & 0xFF;     /* Another multiply */
                
                /* Additional use of the same computed value */
                sum = sum ^ (temp & 0xF);     /* Bitwise operation */
            } else {
                /* Boundary case */
                a[i] = (b_val * 2) & 0xFF;
            }
            
            /* More operations to create scheduling pressure */
            int extra = (a_val ^ b_val) | (product & 0xFF);
            sum = sum - (extra % 16);         /* Modulo operation */
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional based on random to prevent optimization */
        if (rand() % 2) {
            total_sum ^= 1;  /* Minor perturbation */
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE], b[SIZE];
    int i;
    
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call the loop function */
    long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    /* Additional print to use arrays */
    printf("First few a[] after processing: %d %d %d\n", a[0], a[1], a[2]);
    
    return 0;
}
