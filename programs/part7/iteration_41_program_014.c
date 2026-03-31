/* sel-sched-trigger.c
 * Designed to trigger GCC's selective scheduler verbose debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-verbose=5 -fdump-rtl-all -fno-schedule-insns -fno-schedule-insns2 -march=native -fno-pie sel-sched-trigger.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1000

/* External function to prevent inlining and create more complex control flow */
extern int external_helper(int x);

/* Simple external function that compiler can't analyze */
int external_helper(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

/* Function with complex loop structure to trigger selective scheduling */
void process_arrays(int n, int* arr1, int* arr2, float* farr1, float* farr2, 
                   double* darr1, double* darr2) {
    int i, j;
    volatile int loop_limit = n;  /* Prevent constant propagation */
    int acc = 0;                  /* Loop-carried dependency */
    float f_acc = 0.0f;
    double d_acc = 0.0;
    
    /* Outer loop to give scheduler repeated region */
    for (j = 0; j < 3; j++) {
        /* Reset accumulators each outer iteration */
        acc = external_helper(j);
        f_acc = (float)j * 0.1f;
        d_acc = (double)j * 0.01;
        
        /* Main computational loop with mixed operations */
        for (i = 1; i < loop_limit; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed floating-point operations */
            float temp_f = farr1[i] * 2.0f - farr1[i-1];
            f_acc += temp_f;
            
            /* Double precision computation */
            double temp_d = darr1[i] / (darr2[i] + 1.0);
            d_acc += temp_d;
            
            /* Non-trivial array indexing */
            int idx = (i * 3) % ARRAY_SIZE;
            int idx2 = (i * 5) % ARRAY_SIZE;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f && (arr1[i] & 0x3) == 0) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i])) + f_acc * 0.1f;
                
                /* Integer operation with bitwise manipulation */
                arr1[idx] = (arr1[idx] & 0xFF00FF) | 
                           ((arr2[idx2] & 0xFF) << 8);
            } else {
                /* Alternative computation path */
                farr2[i] = farr1[i] * farr1[i-1] - f_acc * 0.05f;
                
                /* Different bitwise operation */
                arr1[idx] = (arr1[idx] ^ arr2[idx2]) & 0xFFFF;
            }
            
            /* More mixed operations outside condition */
            darr2[i] = d_acc * 0.01 + sin((double)i * 0.01);
            
            /* Integer operation with floating-point conversion */
            arr2[i] = (int)(farr2[i] * 100.0f) + acc % 100;
            
            /* Additional conditional with floating-point comparison */
            if (darr1[i] > darr2[i] * 2.0) {
                /* Nested condition increases control flow complexity */
                arr1[i] = arr1[i] + (int)(darr1[i] - darr2[i]);
            }
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        arr1[0] += acc;
        farr1[0] += f_acc;
        darr1[0] += d_acc;
    }
}

/* Another function with different pattern to increase scheduling complexity */
void process_with_stride(int n, int* data, float* fdata, int stride) {
    int i;
    volatile int limit = n;
    int sum = 0;
    float fsum = 0.0f;
    
    for (i = 0; i < limit; i += stride) {
        /* Memory accesses with stride pattern */
        int val1 = data[i];
        int val2 = data[(i + stride) % ARRAY_SIZE];
        float fval = fdata[i];
        
        /* Complex integer arithmetic */
        int combined = (val1 * val2) + (val1 >> 3) - (val2 << 2);
        sum += combined;
        
        /* Floating-point with type conversion */
        fsum += fval * (float)combined;
        
        /* Store with stride */
        data[i] = combined % 256;
        fdata[i] = fsum * 0.01f;
        
        /* Conditional with floating-point */
        if (fsum > 100.0f) {
            data[i] = data[i] | 0x80000000;
            fdata[i] = -fdata[i];
        }
    }
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > ARRAY_SIZE) n = ARRAY_SIZE;
    if (n < 10) n = 10;
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* farr1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float* farr2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* darr1 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* darr2 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Seed RNG for unpredictable values */
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / (float)RAND_MAX;
        farr2[i] = (float)rand() / (float)RAND_MAX;
        darr1[i] = (double)rand() / (double)RAND_MAX;
        darr2[i] = (double)rand() / (double)RAND_MAX;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_arrays(n, arr1, arr2, farr1, farr2, darr1, darr2);
        
        /* Vary stride to create different access patterns */
        int stride = 1 + (iter % 3);
        process_with_stride(n, arr1, farr1, stride);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    double fchecksum = 0.0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i] + darr1[i] + darr2[i];
    }
    
    printf("Checksum: %ld, Floating checksum: %f\n", 
           (long)checksum, fchecksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(darr1);
    free(darr2);
    
    return 0;
}
