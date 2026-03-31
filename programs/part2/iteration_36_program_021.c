/* sel-sched-test.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with memory aliasing and complex dependencies */
static inline int process_value(int *restrict arr1, int *arr2, 
                               float *restrict farr, double *darr,
                               int idx, int mod) {
    int temp;
    float ftemp;
    double dtemp;
    
    /* Multiple arithmetic operations with dependencies */
    temp = arr1[idx] * 3 + global_seed;
    ftemp = farr[idx] * 2.5f - (float)temp;
    dtemp = darr[idx] / 1.7 + (double)ftemp;
    
    /* Conditional store with potential aliasing */
    if (mod % 7 == 0) {
        arr2[idx] = (int)(dtemp * 1000);
        farr[idx] = ftemp * 0.9f;
    } else if (mod % 5 == 0) {
        arr1[idx] = temp + idx;
        darr[idx] = dtemp - 1.0;
    }
    
    /* Complex return with mixed operations */
    return (int)((temp * ftemp + dtemp) / (idx + 1));
}

/* Hot loop with carried dependencies and multiple basic blocks */
static inline uint64_t compute_loop(int *restrict a, int *b, 
                                   float *restrict f, double *d,
                                   int size, int iter) {
    uint64_t sum = 0;
    int i, j;
    
    /* Outer loop with carried dependency */
    for (j = 0; j < iter; j++) {
        int carry = j;
        
        /* Inner hot loop with complex operations */
        for (i = 0; i < size; i++) {
            /* Multiple independent operations */
            int val1 = a[i] + carry;
            float val2 = f[i] * 1.1f + (float)carry;
            double val3 = d[i] / (1.0 + (double)(i % 3));
            
            /* Conditional branch creating multiple basic blocks */
            if ((i + j) % 11 == 0) {
                val1 = val1 * 2 - b[i];
                val2 = val2 + f[i % size];
                val3 = val3 * 0.8;
            } else if ((i + j) % 13 == 0) {
                val1 = val1 / 2 + b[(i + 1) % size];
                val2 = val2 - 0.5f;
                val3 = val3 + 1.2;
            }
            
            /* Call to function with memory operations */
            int result = process_value(a, b, f, d, i, i + j);
            
            /* Update carried dependency */
            carry = (carry + result) % 1000;
            
            /* Mix integer and floating point in accumulation */
            sum += (uint64_t)(val1 + (int)val2 + (int)val3 + result);
            
            /* Memory store with potential aliasing */
            b[i] = (b[i] + val1) & 0xFFF;
            f[i] = val2 * 0.99f;
            d[i] = val3 * 0.95;
        }
        
        /* Additional dependency chain */
        for (i = size - 1; i >= 0; i--) {
            a[i] = (a[i] + carry) ^ 0x5A5A;
            carry = (carry * 13 + a[i]) % 7919;
        }
    }
    
    return sum;
}

/* Another hot function to encourage inlining */
static inline uint64_t secondary_loop(int *arr, float *farr, int size) {
    uint64_t acc = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Mixed operations */
        float fval = farr[i];
        int ival = arr[i];
        
        /* Division creates complex RTL */
        if (ival != 0) {
            fval = fval / (float)((ival & 0xF) + 1);
        }
        
        /* Complex integer arithmetic */
        ival = ((ival * 3) + (i * 7)) % 1024;
        
        /* Update accumulators */
        arr[i] = ival;
        farr[i] = fval;
        acc += (uint64_t)(ival + (int)(fval * 100));
    }
    
    return acc;
}

int main(void) {
    const int SIZE = 256;
    const int ITERS = 100;
    
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    double *darray = (double*)malloc(SIZE * sizeof(double));
    
    int i;
    for (i = 0; i < SIZE; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 + 2;
        farray[i] = (float)i * 1.7f + 3.14f;
        darray[i] = (double)i * 2.3 + 6.28;
    }
    
    uint64_t total_sum = 0;
    
    /* Call hot loops multiple times */
    for (i = 0; i < 5; i++) {
        total_sum ^= compute_loop(array1, array2, farray, darray, SIZE, ITERS);
        total_sum ^= secondary_loop(array1, farray, SIZE);
        
        /* Modify global to prevent optimization */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final computation to use results */
    uint64_t checksum = 0;
    for (i = 0; i < SIZE; i++) {
        checksum ^= (uint64_t)array1[i];
        checksum ^= (uint64_t)array2[i];
        checksum ^= (uint64_t)(farray[i] * 1000);
        checksum ^= (uint64_t)(darray[i] * 1000);
    }
    checksum ^= total_sum;
    
    printf("Result checksum: %llu\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray);
    free(darray);
    
    return 0;
}
