/* Program to trigger GCC modulo scheduler debug output in ps_insn_find_column */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int n_outer) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < n_outer; j++) {
        /* Inner loop with carried dependency for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex carried dependency: sum depends on previous sum */
            /* Multiple arithmetic operations to create scheduling opportunities */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        /* Prevents complete optimization of outer loop */
        b[0] += sum & 0xF;
        a[31] ^= sum & 0xFF;
    }
    
    return sum;
}

/* Another test function with different dependency pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int i;
    int acc1 = arr[0];
    int acc2 = arr[1];
    
    /* Loop with multiple interleaved recurrences */
    for (i = 2; i < n; i++) {
        /* Two separate carried dependencies */
        acc1 = (acc1 * 3 + arr[i]) & 0x7FFF;
        acc2 = (acc2 + acc1 - arr[i-1]) & 0x7FFF;
        
        /* Cross-dependency between the two accumulators */
        int temp = acc1 ^ acc2;
        acc1 = acc1 + (temp >> 4);
        acc2 = acc2 - (temp & 0xF);
    }
    
    return acc1 + acc2;
}

int main(void) {
    int a[128], b[128];
    int i;
    
    /* Seed random number generator for array initialization */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values */
    /* Using rand() prevents constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call test functions multiple times */
    int result1 = modulo_sched_test(a, b, 10);
    
    /* Re-initialize for second test */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 512;
    }
    
    int result2 = modulo_sched_test2(a, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
