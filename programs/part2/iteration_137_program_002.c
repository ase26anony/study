/* Program to trigger modulo scheduler debug logging in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Inner loop with carried dependency - critical for modulo scheduling */
    for (i = 0; i < size; ++i) {
        /* Complex recurrence with multiple operations */
        sum = (sum * a[i] + b[i]) >> 1;
        /* Additional operations to increase instruction count */
        sum = sum ^ (a[i] & 0xFF);
        sum = sum + (b[i] % 256);
    }
    
    return sum;
}

/* Another loop variant with different dependency pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int *a, int *b, int size) {
    int sum = a[0];  /* Start with first element */
    int i;
    
    /* Different recurrence pattern: a[i] depends on previous iteration */
    for (i = 1; i < size; ++i) {
        int temp = sum * 3;
        a[i] = temp + b[i];
        sum = a[i] - (sum >> 2);
        /* More arithmetic operations */
        sum = sum * 2 + (i & 0xF);
    }
    
    return sum;
}

int main(void) {
    const int ARRAY_SIZE = 64;
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    volatile int seed = time(NULL);  /* Prevent constant propagation */
    int i, j;
    int total_sum = 0;
    
    /* Initialize arrays with pseudo-random values */
    srand(seed);
    for (i = 0; i < ARRAY_SIZE; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 20; ++j) {
        /* Call first loop variant */
        total_sum += compute_loop(a, b, 32);  /* Fixed small size */
        
        /* Modify arrays slightly to create loop-variant behavior */
        a[0] += total_sum & 0xFF;
        b[j % ARRAY_SIZE] = total_sum % 100;
        
        /* Call second loop variant */
        total_sum += compute_loop2(a, b, 32);
        
        /* More array modifications */
        if (j % 3 == 0) {
            for (i = 0; i < ARRAY_SIZE; i += 4) {
                b[i] = (b[i] + a[i]) & 0x3FF;
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total_sum);
    return total_sum & 0xFF;  /* Return non-zero to indicate success */
}
