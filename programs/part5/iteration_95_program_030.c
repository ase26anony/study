#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        /* Create loop-carried dependency with distance 1 */
        int prev = a[0];  /* Initialize for first iteration */
        
        /* Inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int load_a = a[i];          /* Load operation */
            int load_b = b[i];          /* Load operation */
            
            /* Integer multiply (higher latency on x86) */
            int product = load_a * load_b;
            
            /* Loop-carried dependency: sum accumulates across iterations */
            sum = sum + product;        /* Distance-0 use within iteration */
            
            /* Create distance-1 dependency: use value from previous iteration */
            int temp = prev + load_b;   /* prev is from i-1 iteration */
            
            /* More operations to create scheduling complexity */
            temp = temp & 0xFF;         /* Bitwise operation */
            temp = temp ^ product;      /* Another bitwise operation */
            
            /* Store with distance-1 dependency */
            a[i] = temp;                /* Store for next iteration's use */
            prev = temp;                /* Update for next iteration */
            
            /* Additional distance-0 uses of the same value */
            if (temp > 1000) {          /* Conditional creates control flow */
                sum = sum - (temp >> 2); /* Arithmetic shift */
            }
            
            /* Another multiply to increase pressure */
            int extra = load_a * (i & 0x3F);
            sum = sum + (extra & 0xFF);
        }
        
        outer_counter++;  /* Volatile prevents optimization */
        
        /* Add some control flow variability */
        if (rand() % 2) {
            /* Swap arrays occasionally */
            int *tmp = a;
            a = b;
            b = tmp;
        }
    }
    
    return sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i, result;
    
    /* Seed random number generator */
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
    
    /* Also print some array values to ensure stores aren't optimized away */
    printf("Sample values: a[0]=%d, a[100]=%d, b[0]=%d\n", 
           a[0], a[100], b[0]);
    
    return 0;
}
