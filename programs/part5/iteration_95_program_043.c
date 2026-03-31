#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    int i, j;
    int sum = 0;
    int temp;
    
    /* Outer loop with volatile control */
    for (j = 0; j < OUTER_ITERATIONS; j++) {
        outer_counter = j;  /* Force memory access */
        
        /* Inner loop with complex dependencies */
        /* True loop-carried dependency: sum depends on previous iteration */
        for (i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int product = a[i] * b[i];        /* Multi-cycle multiply */
            sum = sum + product;              /* Distance-1 dependency on sum */
            
            /* Additional operations to create scheduling pressure */
            int diff = a[i] - b[i];           /* Simple subtract */
            int bitwise = product & diff;     /* Bitwise operation */
            
            /* Loop-carried dependency through array */
            if (i > 0) {
                /* Distance-1 dependency: a[i] depends on a[i-1] */
                a[i] = a[i-1] + bitwise;      /* Distance-1 use */
            } else {
                a[i] = bitwise;               /* No dependency for first element */
            }
            
            /* Another use of product to create multiple uses */
            temp = product >> 2;              /* Shift operation */
            sum = sum ^ temp;                 /* Another dependency on sum */
            
            /* Complex expression with multiple operations */
            b[i] = (b[i] * 3) + (a[i] & 0xFF); /* Mix of operations */
        }
        
        /* Conditional to prevent optimization */
        if (rand() % 2) {
            sum = sum * 2;  /* Modify sum occasionally */
        }
    }
    
    return sum;
}

int main() {
    int a[SIZE], b[SIZE];
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
    
    /* Also print a few array values to ensure they're used */
    printf("Sample values: a[0]=%d, b[0]=%d\n", a[0], b[0]);
    
    return 0;
}
