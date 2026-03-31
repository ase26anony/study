/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        for (i = 0; i < size; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication creates multi-cycle latency
               - Addition provides another operation
               - Shift breaks pattern for scheduling challenge
               This creates a true data dependency chain across iterations */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (i & 0x3);
        }
        
        /* Modify input slightly to prevent complete optimization */
        if (outer % 3 == 0) {
            b[0] += sum;
        }
    }
    
    return sum;
}

/* Another function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int *arr, int size) {
    int acc1 = 5, acc2 = 7;
    
    for (int outer = 0; outer < 5; outer++) {
        /* Dual recurrence chains for more complex scheduling */
        for (int i = 0; i < size; i++) {
            acc1 = (acc1 * 3 + arr[i]) & 0xFFF;
            acc2 = (acc2 + acc1 * 2) >> 1;
            arr[i] = acc1 + acc2;  /* Store result creates memory dependency */
        }
        
        /* Cross-iteration dependency */
        arr[0] = acc1 + acc2;
    }
    
    return acc1 + acc2;
}

int main() {
    const int SIZE = 32;  /* Small, constant size for modulo scheduling */
    int a[SIZE], b[SIZE];
    int arr[SIZE];
    
    /* Initialize with volatile-like behavior to prevent constant propagation */
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100 + 1;
        b[i] = rand() % 100 + 1;
        arr[i] = rand() % 50;
    }
    
    /* Use volatile variable to prevent optimization */
    volatile int seed = rand();
    a[0] += seed;
    b[0] += seed;
    
    printf("Starting computation...\n");
    
    /* Call both functions to increase chance of triggering the scheduler */
    int result1 = compute_loop(a, b, SIZE);
    int result2 = compute_loop2(arr, SIZE);
    
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    return 0;
}
