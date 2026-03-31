/* Compile with: gcc -O2 -fmodulo-sched -fno-tree-vectorize -fno-unroll-loops -fdump-rtl-sms -fdump-rtl-sms-details modulo_test.c -o modulo_test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent unwanted optimizations */
#define NO_OPT __attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))

/* Function with loop designed for modulo scheduling */
NO_OPT int modulo_scheduled_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Outer loop to increase scheduling analysis opportunities */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count (32) for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple arithmetic operations */
            /* sum depends on previous sum value (carried dependency) */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        b[0] += sum % 100;
        a[1] ^= sum;
    }
    
    return sum;
}

/* Another variant with different recurrence pattern */
NO_OPT int another_modulo_loop(int *arr1, int *arr2, int n) {
    int acc = arr1[0];
    int temp;
    
    /* Loop with multiple carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        temp = acc * 3;
        acc = temp + arr1[i];
        acc = acc - arr2[i-1];
        acc = (acc << 2) | (acc >> 30);  /* Rotate */
        arr2[i] = acc % 256;
    }
    
    return acc;
}

int main() {
    int i;
    int result1, result2;
    
    /* Initialize with volatile-like behavior using rand() */
    srand(time(NULL));
    
    /* Arrays with pseudo-random values to prevent constant propagation */
    int array_a[128];
    int array_b[128];
    int array_c[64];
    int array_d[64];
    
    /* Initialize arrays with varying values */
    for (i = 0; i < 128; i++) {
        array_a[i] = rand() % 1000;
        array_b[i] = rand() % 1000;
    }
    
    for (i = 0; i < 64; i++) {
        array_c[i] = rand() % 500;
        array_d[i] = rand() % 500;
    }
    
    /* Execute modulo-scheduled loops */
    result1 = modulo_scheduled_loop(array_a, array_b, 32);
    result2 = another_modulo_loop(array_c, array_d, 32);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Additional test with different sizes */
    for (int iter = 0; iter < 5; iter++) {
        int small_a[16];
        int small_b[16];
        
        for (i = 0; i < 16; i++) {
            small_a[i] = rand() % 100;
            small_b[i] = rand() % 100;
        }
        
        int small_result = modulo_scheduled_loop(small_a, small_b, 16);
        printf("Iteration %d: %d\n", iter, small_result);
    }
    
    return 0;
}
