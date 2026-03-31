/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void modulo_sched_loop(int *a, int *b, int n) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int temp;  /* Prevent optimizations */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency for modulo scheduling */
        for (int i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 256);
            
            /* Use volatile to prevent dead code elimination */
            temp = sum;
        }
        
        /* Modify input slightly for outer loop variation */
        b[0] += sum;
        a[outer % 32] = sum & 0xFFFF;
    }
    
    /* Use the result to prevent elimination */
    printf("Final sum: %d\n", sum);
}

/* Another function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void second_modulo_loop(int *arr, int n) {
    int acc = arr[0];
    
    for (int iter = 0; iter < 5; iter++) {
        /* Loop with multiple carried dependencies */
        for (int i = 1; i < 64; i++) {
            /* Multiple recurrence chains */
            int t1 = acc * arr[i];
            int t2 = t1 + (arr[i-1] << 2);
            acc = (t2 ^ (i * 3)) + 1;
            
            /* Cross-iteration dependency */
            arr[i] = acc + arr[i-1];
        }
        
        /* Break potential pattern */
        acc = acc ^ (iter * 0x1234);
    }
    
    printf("Second accumulator: %d\n", acc);
}

int main(void) {
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    
    int a[128], b[128];
    
    /* Fill arrays with varying values */
    for (int i = 0; i < 128; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the modulo-scheduled function */
    modulo_sched_loop(a, b, 32);
    
    /* Call second function with different data */
    int arr[128];
    for (int i = 0; i < 128; i++) {
        arr[i] = rand() % 500;
    }
    second_modulo_loop(arr, 64);
    
    return 0;
}
