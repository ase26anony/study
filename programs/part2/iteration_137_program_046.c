/* Program to trigger modulo scheduling debug logging in GCC */
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
            /* Complex recurrence with multiple arithmetic operations */
            /* This creates true data dependencies between iterations */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 256);
        }
        
        /* Modify input slightly to prevent complete optimization */
        /* This creates loop-variant behavior for outer loop */
        if (j % 2 == 0) {
            b[0] += sum;
        } else {
            a[0] ^= sum;
        }
    }
    
    return sum;
}

int main() {
    int a[128], b[128];
    int i, result;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values */
    /* Using volatile-like behavior to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the test function multiple times */
    result = modulo_sched_test(a, b, 128);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with different patterns */
    for (i = 0; i < 128; i++) {
        a[i] = i * 3;
        b[i] = i * 7 + 1;
    }
    
    result = modulo_sched_test(a, b, 128);
    printf("Result2: %d\n", result);
    
    return 0;
}
