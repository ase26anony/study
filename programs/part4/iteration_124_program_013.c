/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Function with complex mixed operations */
__attribute__((noinline, optimize("O3")))
int test_mixed_operations(int* arr, int n, float* farr) {
    int int_sum = 0;
    float float_sum = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing mode */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : (val * 2);
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = farr[i];
        float fprod = helper_fmul(fval, i * 0.1f);
        
        /* Mixed-type computation */
        int_sum += prod + cond_val + popcnt;
        float_sum += fprod;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            int_sum += barrier;
            /* Another memory store */
            arr[i] = int_sum % 256;
        } else if (i % 5 == 0) {
            float_sum += barrier * 0.5f;
        }
        
        /* Additional arithmetic to create ILP */
        int_sum = (int_sum * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final computation mixing int and float */
    return int_sum + (int)float_sum;
}

/* Function targeting specific architecture */
__attribute__((target("arch=core2"), noinline, optimize("O3")))
int test_vectorized_ops(short* sarr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i += 4) {
        /* Operations that might vectorize */
        short s0 = sarr[i];
        short s1 = sarr[i + 1];
        short s2 = sarr[i + 2];
        short s3 = sarr[i + 3];
        
        /* Mixed-width operations */
        int ext0 = (int)s0 * s0;
        int ext1 = (int)s1 * s1;
        int ext2 = (int)s2 * s2;
        int ext3 = (int)s3 * s3;
        
        /* Complex expression with multiple uses */
        sum += ext0 + ext1 + ext2 + ext3;
        sum = (sum << 3) | (sum >> 29); /* Rotate */
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Conditional store */
        if (sum > 1000000) {
            sarr[i] = (short)(sum & 0xFFFF);
        }
    }
    
    return sum;
}

/* Outer loop with pipelining opportunities */
__attribute__((noinline, optimize("O3")))
int test_outer_loop_pipelining(int* data, int size) {
    int total = 0;
    
    for (int outer = 0; outer < 10; outer++) {
        int inner_sum = 0;
        
        /* Inner loop with data-dependent computation */
        for (int i = 0; i < size; i++) {
            /* Complex addressing */
            int idx = (i + outer) % size;
            int val = data[idx];
            
            /* Multiple independent operations */
            int a = val * 3;
            int b = val + 7;
            int c = val ^ 0xFF;
            
            /* All used in final computation */
            inner_sum += a * b + c;
            
            /* Memory store with dependency */
            data[idx] = inner_sum & 0xFF;
        }
        
        total += inner_sum;
        
        /* Barrier between outer loop iterations */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Function with pointer chasing and complex dependencies */
__attribute__((noinline, optimize("O3")))
int test_pointer_chasing(int** ptr_array, int n) {
    int sum = 0;
    int* current = ptr_array[0];
    
    for (int i = 0; i < n; i++) {
        /* Load from pointer */
        int val = *current;
        
        /* Complex computation */
        int transformed = ((val << 4) ^ (val >> 4)) + i;
        
        /* Conditional update */
        if (transformed % 2 == 0) {
            sum += transformed;
            current = ptr_array[(i + 1) % n];
        } else {
            sum -= transformed;
            current = &ptr_array[i % n][0];
        }
        
        /* Another barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 256;
    
    /* Allocate and initialize test data */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    short* short_array = (short*)malloc(SIZE * sizeof(short));
    int** ptr_array = (int**)malloc(SIZE * sizeof(int*));
    
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 37) & 0xFF;
        float_array[i] = i * 0.1f;
        short_array[i] = (short)(i * 13);
        ptr_array[i] = &int_array[i];
    }
    
    /* Call test functions to generate diverse RTL patterns */
    int result1 = test_mixed_operations(int_array, SIZE, float_array);
    int result2 = test_vectorized_ops(short_array, SIZE);
    int result3 = test_outer_loop_pipelining(int_array, SIZE / 2);
    int result4 = test_pointer_chasing(ptr_array, SIZE / 4);
    
    /* Final result to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(short_array);
    free(ptr_array);
    
    return 0;
}
