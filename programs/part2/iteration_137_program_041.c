/* Program to trigger GCC modulo scheduler debug logging in ps_insn_find_column */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Inner loop with carried dependency - critical for modulo scheduling */
    for (i = 0; i < size; ++i) {
        /* Complex recurrence with multiple operations */
        sum = (sum * a[i] + b[i]) >> 1;
        
        /* Additional operations to increase instruction count */
        sum = sum ^ (a[i] & 0xFF);
        sum = sum + (b[i] % 256);
        
        /* Another carried dependency chain */
        if (i > 0) {
            sum = sum - (a[i-1] & 0x7F);
        }
    }
    
    return sum;
}

/* Outer loop to provide multiple scheduling contexts */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int outer_loop_driver() {
    volatile int seed = time(NULL);  /* Prevent constant propagation */
    int a[128], b[128];
    int i, j;
    int total_sum = 0;
    
    /* Initialize arrays with pseudo-random values */
    srand(seed);
    for (i = 0; i < 128; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Outer loop - forces scheduler to analyze inner loop multiple times */
    for (j = 0; j < 10; ++j) {
        /* Call the inner loop with carried dependency */
        total_sum += modulo_sched_loop(a, b, 32);
        
        /* Modify input arrays slightly to create loop-variant behavior */
        b[0] += total_sum & 0xFF;
        a[j % 32] = (a[j % 32] * 13 + 17) % 997;
    }
    
    return total_sum;
}

int main() {
    int result;
    
    printf("Starting modulo scheduling test...\n");
    
    /* Execute the test multiple times */
    for (int run = 0; run < 3; ++run) {
        result = outer_loop_driver();
        printf("Run %d: Result = %d\n", run, result);
    }
    
    /* Additional test with different patterns */
    {
        int arr1[64], arr2[64];
        int acc = 5;
        
        /* Initialize with alternating pattern */
        for (int i = 0; i < 64; i++) {
            arr1[i] = i * 3;
            arr2[i] = i * 7 + 1;
        }
        
        /* Another carried dependency loop */
        for (int i = 0; i < 32; i++) {
            acc = acc * arr1[i] + arr2[i];
            acc = (acc << 3) | (acc >> 29);  /* Rotation */
            if (i > 0) {
                acc = acc ^ arr1[i-1];
            }
        }
        
        printf("Additional test result: %d\n", acc);
    }
    
    return 0;
}
