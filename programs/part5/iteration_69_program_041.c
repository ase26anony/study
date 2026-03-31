/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float* restrict a, float* restrict b, float* restrict c, 
                        float* restrict d, float* restrict e, float* restrict f) {
    /* Many independent computations to create multiple scheduling candidates */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i] + c[i];
        float t2 = d[i] * e[i] - f[i];
        float t3 = a[i] / (b[i] + 1.0f);
        float t4 = c[i] * d[i] / e[i];
        
        /* Group 2: More independent operations */
        float t5 = t1 + t2 * t3;
        float t6 = t4 - t1 / t2;
        float t7 = t5 * t6 + t3;
        float t8 = t4 / t5 - t6;
        
        /* Group 3: Integer operations mixed with FP */
        int it1 = (int)(t1 * 1000);
        int it2 = (int)(t2 * 1000);
        int it3 = it1 * it2 + i;
        int it4 = it1 / (it2 + 1) * i;
        
        /* Create artificial dependencies with volatile */
        t1 += vol_float1;
        t2 -= vol_float2;
        it3 += vol_var1;
        it4 -= vol_var2;
        
        /* Store results creating memory pressure */
        a[i] = t1 + t5;
        b[i] = t2 + t6;
        c[i] = t3 + t7;
        d[i] = t4 + t8;
        e[i] = (float)it3 + t1;
        f[i] = (float)it4 + t2;
    }
}

/* Function with mixed dependencies and potential delays */
__attribute__((noinline))
void mixed_dependency_ops(double* restrict arr1, double* restrict arr2, 
                          int* restrict idx, int size) {
    /* Long latency operations */
    for (int i = 0; i < size; i++) {
        /* FP divide - long latency */
        double d1 = arr1[i] / 3.14159265358979;
        
        /* Memory access with potential aliasing */
        double d2 = arr2[idx[i]] * 2.71828182845904;
        
        /* Dependency chain */
        double d3 = d1 * d1 + d2;
        double d4 = d2 * d2 - d1;
        
        /* Volatile read creates scheduling barrier */
        int v = vol_var1;
        
        /* Conditional creates control flow for priority differences */
        if (d3 > d4) {
            d3 = d3 * v;
        } else {
            d4 = d4 / (v + 1);
        }
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile (
            "mov $0x1, %%eax\n\t"
            "mov $0x2, %%ebx\n\t"
            "mov $0x3, %%ecx\n\t"
            "mov $0x4, %%edx\n\t"
            "mov $0x5, %%esi\n\t"
            "mov $0x6, %%edi\n\t"
            :
            :
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Store results */
        arr1[i] = d3 + d4;
        arr2[i] = d3 - d4;
    }
}

/* Function with completely independent instruction groups */
__attribute__((noinline))
void independent_groups(int* restrict out1, int* restrict out2, 
                        int* restrict out3, int* restrict out4, int n) {
    for (int i = 0; i < n; i++) {
        /* Group A - independent computations */
        int a1 = i * 3 + 1;
        int a2 = i * 5 - 2;
        int a3 = a1 * a2;
        int a4 = a1 + a2 * 3;
        
        /* Group B - independent from Group A */
        int b1 = i * 7 + 3;
        int b2 = i * 11 - 5;
        int b3 = b1 * b2 / 2;
        int b4 = b1 + b2 * 7;
        
        /* Group C - more independence */
        int c1 = a3 * b3;
        int c2 = a4 + b4;
        int c3 = c1 / (c2 + 1);
        int c4 = c1 * c2 - c3;
        
        /* Volatile operations create scheduling points */
        a3 += vol_var1;
        b3 -= vol_var2;
        c3 ^= vol_var1;
        
        /* Store to different arrays to prevent aliasing */
        out1[i] = a3;
        out2[i] = b3;
        out3[i] = c3;
        out4[i] = c4;
    }
}

/* Main computational kernel */
__attribute__((noinline))
void compute_kernel(float* farr1, float* farr2, double* darr1, double* darr2,
                    int* iarr1, int* iarr2, int* iarr3, int* iarr4) {
    /* Mix different types of operations to create varied scheduling priorities */
    
    /* 1. High register pressure section */
    high_pressure_loop(farr1, farr2, farr1 + ARRAY_SIZE/2, 
                       farr2 + ARRAY_SIZE/2, farr1 + ARRAY_SIZE/4, 
                       farr2 + ARRAY_SIZE/4);
    
    /* 2. Create index array for memory aliasing */
    int indices[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        indices[i] = (i * 7) % ARRAY_SIZE;
    }
    
    /* 3. Mixed dependency operations with potential delays */
    mixed_dependency_ops(darr1, darr2, indices, ARRAY_SIZE);
    
    /* 4. Independent instruction groups */
    independent_groups(iarr1, iarr2, iarr3, iarr4, ARRAY_SIZE);
    
    /* 5. Final reduction with dependency chain */
    double sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += darr1[i] + darr2[i] + farr1[i] + farr2[i] + 
               iarr1[i] + iarr2[i] + iarr3[i] + iarr4[i];
    }
    
    /* Use result to prevent dead code elimination */
    vol_var1 = (int)(fabs(sum) * 0.001);
}

int main() {
    /* Allocate and initialize data */
    float* farr1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farr2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* darr1 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* darr2 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* iarr1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr4 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr1[i] = (float)rand() / RAND_MAX * 100.0f;
        farr2[i] = (float)rand() / RAND_MAX * 100.0f;
        darr1[i] = (double)rand() / RAND_MAX * 100.0;
        darr2[i] = (double)rand() / RAND_MAX * 100.0;
        iarr1[i] = rand() % 1000;
        iarr2[i] = rand() % 1000;
        iarr3[i] = rand() % 1000;
        iarr4[i] = rand() % 1000;
    }
    
    /* Run kernel multiple times to ensure scheduler sees hot code */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_kernel(farr1, farr2, darr1, darr2, iarr1, iarr2, iarr3, iarr4);
        
        /* Modify volatile occasionally to change dependencies */
        if (iter % 1000 == 0) {
            vol_var1 ^= 1;
            vol_var2 ^= 2;
            vol_float1 += 0.1f;
            vol_float2 -= 0.1f;
        }
    }
    
    /* Final checksum to use all results */
    long long total = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += (long long)(farr1[i] + farr2[i] + darr1[i] + darr2[i] + 
                            iarr1[i] + iarr2[i] + iarr3[i] + iarr4[i]);
    }
    
    printf("Result checksum: %lld\n", total);
    
    /* Cleanup */
    free(farr1);
    free(farr2);
    free(darr1);
    free(darr2);
    free(iarr1);
    free(iarr2);
    free(iarr3);
    free(iarr4);
    
    return 0;
}
