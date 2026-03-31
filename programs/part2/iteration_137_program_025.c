/* Program to trigger GCC modulo scheduler debug logging */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int* a, int* b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Inner loop with carried dependency - critical for modulo scheduling */
    for (i = 0; i < size; ++i) {
        /* Complex recurrence with multiple operations */
        sum = (sum * a[i] + b[i]) >> 1;
        
        /* Additional operations to increase instruction count */
        sum ^= (a[i] & 0xFF);
        sum += (b[i] % 17);
    }
    
    return sum;
}

/* Another function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int* a, int* b, int size) {
    int sum = a[0];
    int i;
    
    /* Different recurrence: a[i] depends on previous iteration */
    for (i = 1; i < size; ++i) {
        a[i] = a[i-1] + b[i] * sum;
        sum = (sum + a[i]) & 0x7FFF;
    }
    
    return sum;
}

int main() {
    const int ARRAY_SIZE = 64;
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    volatile int seed = time(NULL);  /* Prevent constant propagation */
    int i, j;
    int total_sum = 0;
    
    srand(seed);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Outer loop to increase scheduling analysis opportunities */
    for (j = 0; j < 10; ++j) {
        /* Call first computation function */
        total_sum += compute_loop(a, b, 32);  /* Fixed small iteration count */
        
        /* Modify arrays slightly to create loop-variant behavior */
        b[0] += total_sum & 0xF;
        a[j % ARRAY_SIZE] ^= total_sum;
        
        /* Call second computation function */
        total_sum += compute_loop2(a, b, 32);
        
        /* Additional array modification */
        if (j % 3 == 0) {
            for (i = 0; i < ARRAY_SIZE; i += 4) {
                b[i] = (b[i] + a[(i+1) % ARRAY_SIZE]) % 256;
            }
        }
    }
    
    printf("Final result: %d\n", total_sum);
    return total_sum & 0xFF;  /* Return non-zero to prevent optimization */
}
