/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int temp;  /* Prevent optimization */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Critical inner loop with carried dependency */
        /* Fixed small iteration count for modulo scheduling */
        for (int i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            /* sum = (sum * a[i] + b[i]) >> 1; */
            /* Alternative: sum = a[i] * sum + b[i] - sum; */
            int idx = i % size;
            sum = (sum * a[idx] + b[idx]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[idx] & 0xFF);
            sum = sum + (b[idx] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        if (outer % 3 == 0) {
            b[0] += sum;
        }
        
        /* Use volatile to prevent dead code elimination */
        temp = sum;
    }
    
    return sum;
}

/* Another test function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 5, acc2 = 7;
    
    for (int iter = 0; iter < 8; iter++) {
        /* Loop with multiple interleaved recurrences */
        for (int i = 0; i < 64; i++) {
            int idx = i % n;
            
            /* Two independent but interacting recurrences */
            acc1 = (acc1 * 3 + arr[idx]) % 1000;
            acc2 = (acc2 + acc1 * arr[(idx + 1) % n]) >> 1;
            
            /* Cross-coupling */
            int tmp = acc1 - acc2;
            acc1 = acc1 ^ (tmp & 0xF);
            acc2 = acc2 | (tmp & 0xF0);
        }
        
        /* Modify array based on accumulated values */
        arr[iter % n] = (arr[iter % n] + acc1) % 256;
    }
    
    return acc1 + acc2;
}

int main() {
    const int SIZE = 128;
    int a[SIZE], b[SIZE];
    int arr[SIZE];
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100 + 1;
        b[i] = rand() % 100 + 1;
        arr[i] = rand() % 256;
    }
    
    /* Call test functions */
    int result1 = modulo_sched_test(a, b, SIZE);
    int result2 = modulo_sched_test2(arr, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
