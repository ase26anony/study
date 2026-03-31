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
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates a challenging scheduling problem */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);  /* XOR with masked value */
            sum = sum + (i & 1);        /* Add loop index parity */
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 0xF;
        a[1] ^= sum;
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 3, acc2 = 5;
    int i, j;
    
    for (j = 0; j < 8; j++) {
        /* Double recurrence with cross dependencies */
        for (i = 1; i < n; i++) {
            /* acc1 depends on previous acc1 and acc2 */
            acc1 = (acc1 * 7 + arr[i] + acc2) % 100;
            
            /* acc2 depends on previous acc2 and current acc1 */
            acc2 = (acc2 * 3 + arr[i-1] + acc1) % 100;
            
            /* Additional arithmetic to increase instruction mix */
            arr[i] = (arr[i] + (acc1 << 2)) & 0xFF;
        }
        
        /* Prevent optimization */
        arr[0] = (arr[0] + j) & 0xFF;
    }
    
    return acc1 + acc2;
}

/* Simple test with just one recurrence */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int simple_recurrence(int *data, int n) {
    int result = 1;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Simple but strong carried dependency */
        result = result * 3 + data[i];
        
        /* Make it non-trivial for the scheduler */
        if (result < 0) result = -result;
        result = result % 1000;
    }
    
    return result;
}

int main() {
    int i;
    int result1, result2, result3;
    
    /* Seed random number generator for array initialization */
    srand(time(NULL));
    
    /* Declare and initialize arrays with volatile-like behavior */
    int a[128], b[128], c[64];
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    for (i = 0; i < 64; i++) {
        c[i] = rand() % 100;
    }
    
    /* Call test functions */
    result1 = modulo_sched_test(a, b, 32);
    result2 = modulo_sched_test2(c, 32);
    result3 = simple_recurrence(a, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return 0;
}
