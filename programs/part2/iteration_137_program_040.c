/* Compile with: gcc -O2 -fmodulo-sched -fno-tree-vectorize -fno-unroll-loops -fdump-rtl-sms -o modulo_test modulo_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count (32) for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple arithmetic operations */
            /* sum depends on previous sum value - creates true data dependency */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior for outer loop */
        b[0] += sum & 0xF;
        a[1] ^= sum;
    }
    
    return sum;
}

int main() {
    int i;
    int a[128], b[128];
    
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int result = compute_loop(a, b, 128);
    
    printf("Result: %d\n", result);
    
    return 0;
}
