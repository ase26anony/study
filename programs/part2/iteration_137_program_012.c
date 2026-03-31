/* Program to trigger uncovered lines in GCC's modulo-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void modulo_sched_loop(int *a, int *b, int n_outer) {
    volatile int sum = 1;  /* Use volatile to prevent optimization */
    int i, j;
    
    for (j = 0; j < n_outer; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count (32) for manageable scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates a challenging scheduling problem */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);  /* XOR operation */
            sum += (i & 1);        /* Conditional-like addition */
        }
        
        /* Modify input slightly to create outer loop variation */
        if (j % 2 == 0) {
            b[0] += sum;
        } else {
            a[0] ^= sum;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final sum: %d\n", sum);
}

int main() {
    int a[128], b[128];
    int i;
    
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values
       to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the function with outer loop iteration count */
    modulo_sched_loop(a, b, 10);
    
    return 0;
}
