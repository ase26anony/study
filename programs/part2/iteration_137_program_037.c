/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int* a, int* b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    
    /* Outer loop to increase scheduling analysis opportunities */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling problem */
        for (int i = 0; i < 32; i++) {
            /* Complex arithmetic with true data dependency across iterations */
            /* sum depends on its previous value -> creates recurrence cycle */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior for outer loop */
        b[0] += sum;
        a[1] ^= sum;
    }
    
    return sum;
}

/* Another function with different pattern to increase coverage chances */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int* arr1, int* arr2, int size) {
    int acc1 = 5, acc2 = 3;
    
    for (int iter = 0; iter < 8; iter++) {
        /* Loop with multiple interleaved dependencies */
        for (int i = 0; i < 64; i++) {
            /* Two parallel recurrence chains */
            acc1 = (acc1 * 3 + arr1[i]) % 1000;
            acc2 = (acc2 + acc1 * arr2[i]) & 0x7FF;
            
            /* Cross-dependency between the two chains */
            int temp = acc1 ^ acc2;
            acc1 = (acc1 + temp) >> 1;
            acc2 = (acc2 - temp) & 0x3FF;
        }
        
        /* Modify arrays to prevent complete optimization */
        arr1[iter % 16] = acc1;
        arr2[iter % 16] = acc2;
    }
    
    return acc1 + acc2;
}

int main() {
    srand(time(NULL));
    
    /* Use volatile to prevent constant propagation */
    volatile int init = rand();
    
    /* Initialize arrays with pseudo-random values */
    int a[128], b[128];
    for (int i = 0; i < 128; i++) {
        a[i] = (rand() % 256) + init;
        b[i] = (rand() % 512) - 256;
    }
    
    /* Call the computation functions */
    int result1 = compute_loop(a, b, 32);
    
    /* Re-initialize for second function */
    int arr1[128], arr2[128];
    for (int i = 0; i < 128; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 200;
    }
    
    int result2 = compute_loop2(arr1, arr2, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
