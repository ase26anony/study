/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication creates multi-cycle latency
               - Addition provides another operation
               - Shift breaks potential pattern recognition
               This creates a good candidate for instruction movement */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);  /* Bitwise operation for variety */
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 1;
    }
    
    return sum;
}

int main() {
    int a[128], b[128];
    int i, result;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values
       Using rand() prevents constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Add some volatile reads to prevent optimization */
    volatile int v = rand();
    a[0] ^= v;
    
    /* Call the test function */
    result = modulo_sched_test(a, b, 128);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
