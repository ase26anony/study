/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int temp;  /* Prevent optimization */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        for (int i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);
            sum += (b[i] % 7);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        temp = sum;
        b[0] += temp;
        a[outer % 32] ^= sum;
    }
    
    return sum;
}

/* Another function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int *a, int *b, int size) {
    int acc = 5;
    
    /* Different loop structure */
    for (int i = 1; i < 64; i++) {
        /* Different type of carried dependency */
        acc = (a[i] * acc + b[i-1]) | 1;
        acc = (acc << 3) ^ b[i];
        
        /* Conditional to add complexity */
        if (acc & 1) {
            acc += a[i] * 3;
        } else {
            acc -= b[i] / 2;
        }
    }
    
    return acc;
}

int main() {
    int a[128], b[128];
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    for (int i = 0; i < 128; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the computation functions */
    int result1 = compute_loop(a, b, 32);
    int result2 = compute_loop2(a, b, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return result1 + result2;
}
