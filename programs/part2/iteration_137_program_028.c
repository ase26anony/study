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
        /* Inner loop with carried dependency for modulo scheduling */
        /* Fixed small iteration count suitable for modulo scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates a good candidate for instruction movement */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);  /* XOR operation */
            sum = sum + (i & 1);        /* Conditional-like addition */
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum;
        a[j % 32] ^= sum;
    }
    
    return sum;
}

/* Another test function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 1, acc2 = 2;
    int i, j;
    
    for (j = 0; j < 5; j++) {
        /* Different recurrence pattern with two accumulators */
        for (i = 1; i < 64; i++) {
            /* Cross-coupled recurrences */
            int temp = acc1;
            acc1 = (acc1 * 3 + arr[i]) / 2;
            acc2 = (acc2 + temp * arr[i-1]) & 0x7FFF;
            
            /* More operations for scheduling complexity */
            acc1 = acc1 ^ (acc2 << 3);
            acc2 = acc2 | (acc1 & 0xFF);
        }
        
        /* Prevent optimization */
        arr[0] = acc1 + acc2;
    }
    
    return acc1 + acc2;
}

int main(void) {
    int a[128], b[128];
    int arr[128];
    int i, result1, result2;
    
    /* Initialize with volatile-like behavior using rand() */
    srand(time(NULL));
    
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 100 + 1;  /* Non-zero values */
        b[i] = rand() % 100 + 1;
        arr[i] = rand() % 256;
    }
    
    /* Call test functions */
    result1 = modulo_sched_test(a, b, 128);
    result2 = modulo_sched_test2(arr, 128);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
