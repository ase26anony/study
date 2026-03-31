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
                        float* restrict d, float* restrict e, int n) {
    /* Many independent calculations to create scheduling candidates */
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i];
        float t2 = c[i] / (d[i] + 1.0f);  /* Division for longer latency */
        float t3 = e[i] * e[i];
        float t4 = t1 + t2;
        float t5 = t3 - t4;
        
        /* Group 2: More independent operations */
        float t6 = sinf(a[i]) * cosf(b[i]);
        float t7 = sqrtf(fabsf(c[i]));
        float t8 = t5 * t6;
        float t9 = t7 + vol_float1;  /* Volatile dependency */
        
        /* Group 3: Integer operations mixed with FP */
        int it1 = (int)(t8 * 1000);
        int it2 = (int)(t9 * 1000);
        int it3 = it1 ^ it2;
        int it4 = it3 * vol_var1;    /* Volatile dependency creates delay */
        
        /* Store results creating register pressure */
        a[i] = t8 + (float)it4 * 0.001f;
        b[i] = t9 - (float)it3 * 0.001f;
        
        /* Artificial dependency chain with volatile */
        if (vol_var2 > 0) {
            c[i] = c[i] * 2.0f - 1.0f;
        }
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int* restrict arr1, int* restrict arr2, 
                      float* restrict farr1, float* restrict farr2, int n) {
    /* Create independent instruction groups for scheduler candidates */
    for (int i = 0; i < n; i++) {
        /* Long latency operations competing for resources */
        float fdiv = farr1[i] / (farr2[i] + 0.0001f);  /* FP division */
        float fsqrt = sqrtf(fabsf(farr1[i]));          /* FP sqrt */
        
        /* Integer operations that can be scheduled independently */
        int imul = arr1[i] * arr2[i];
        int iadd = imul + vol_var1;  /* Volatile creates scheduling boundary */
        
        /* Memory operations with potential aliasing */
        arr1[i] = iadd ^ (int)(fdiv * 1000);
        arr2[i] = (int)(fsqrt * 1000) & 0xFF;
        
        /* Control flow to create priority differences */
        if (i & 1) {
            farr1[i] = fdiv * 2.0f;
        } else {
            farr1[i] = fsqrt * 0.5f;
        }
        
        /* Inline assembly to clobber registers and increase pressure */
        asm volatile("" : : : "memory", "eax", "ebx", "ecx", "edx");
    }
}

/* Function with many independent statements for scheduler to reorder */
__attribute__((noinline))
void independent_instructions(float* restrict a, float* restrict b, 
                              float* restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* These 6 statements are largely independent - scheduler gets many candidates */
        float r1 = a[i] * 3.14159f;
        float r2 = b[i] * 2.71828f;
        float r3 = c[i] * 1.41421f;
        float r4 = r1 + r2;
        float r5 = r2 - r3;
        float r6 = r3 * r4;
        
        /* Cross dependencies to create some ordering constraints */
        a[i] = r4 + r5 * vol_float2;
        b[i] = r5 - r6 / (vol_float1 + 0.1f);
        c[i] = r6 * (1.0f + (float)vol_var2 * 0.01f);
        
        /* Additional independent group */
        float s1 = sinf(r4);
        float s2 = cosf(r5);
        float s3 = s1 * s2;
        a[i] += s3;
    }
}

/* Main computational kernel that exercises all patterns */
__attribute__((noinline))
float compute_kernel(int iterations) {
    /* Allocate arrays with different alignments to potentially affect scheduling */
    float* a = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* b = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* c = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* d = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* e = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    
    int* arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* farr1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float* farr2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = (float)((i + 2) % 100) * 0.3f;
        d[i] = (float)((i + 3) % 100) * 0.4f;
        e[i] = (float)((i + 4) % 100) * 0.5f;
        
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = (float)i * 0.01f;
        farr2[i] = (float)i * 0.02f;
    }
    
    float total = 0.0f;
    
    /* Main computation loop - will be optimized and scheduled */
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix different computation patterns */
        high_pressure_loop(a, b, c, d, e, ARRAY_SIZE);
        mixed_dependency(arr1, arr2, farr1, farr2, ARRAY_SIZE);
        independent_instructions(a, b, c, ARRAY_SIZE);
        
        /* Create dependency between iterations */
        vol_var1 = (vol_var1 * 1103515245 + 12345) & 0x7fffffff;
        vol_var2 = iter % 100;
        
        /* Accumulate results to prevent dead code elimination */
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            total += a[i] + b[i] + c[i] + (float)arr1[i];
        }
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e);
    free(arr1); free(arr2); free(farr1); free(farr2);
    
    return total;
}

int main() {
    clock_t start = clock();
    
    printf("Starting scheduling coverage test...\n");
    
    /* Perform computation - this is where scheduling happens during compilation */
    float result = compute_kernel(ITERATIONS);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %f (checksum to prevent optimization)\n", result);
    printf("Time elapsed: %.2f seconds\n", elapsed);
    printf("Test completed.\n");
    
    return 0;
}
