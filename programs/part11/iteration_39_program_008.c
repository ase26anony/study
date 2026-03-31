/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 * 
 * Compilation flags to trigger coverage:
 *   gcc -O3 -fschedule-insns -fschedule-insns2 -mtune=generic -march=x86-64 -fno-omit-frame-pointer -o test test_scheduler_coverage.c
 * 
 * For ARM targets:
 *   gcc -O2 -fschedule-insns -fschedule-insns2 -mcpu=cortex-a57 -mfpu=neon -o test_arm test_scheduler_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Prevent compiler from optimizing away computations */
static volatile int sink;

/* Inline functions to increase instruction count in basic blocks */
static inline double compute_pressure(double a, double b) {
    return a * b + a / (b + 1.0) - sqrt(a * b);
}

static inline int int_chain(int a, int b, int c) {
    return (a * b + c) ^ (a - b) | (c << 3);
}

/* Function with mixed integer/FP operations and memory aliasing */
void complex_block_1(int *arr, double *darr, int n) {
    int i;
    double acc = 0.0;
    int sum = 0;
    
    /* Wide basic block with unrolled loop - creates large instruction queue */
    for (i = 0; i < n; i += 4) {
        /* Multiple dependent arithmetic operations */
        int t1 = arr[i] * 3 + 7;
        int t2 = t1 - arr[i + 1];
        int t3 = t2 ^ (arr[i + 2] << 2);
        int t4 = t3 | arr[i + 3];
        
        /* Mixed FP operations using different execution units */
        double d1 = darr[i] * 2.5;
        double d2 = d1 + compute_pressure(darr[i + 1], d1);
        double d3 = d2 - sin(darr[i + 2]);
        double d4 = d3 * cos(darr[i + 3]);
        
        /* Memory operations with potential aliasing */
        arr[i] = t4;
        darr[i] = d4;
        
        /* Accumulate results to prevent dead code elimination */
        sum += t4;
        acc += d4;
        
        /* Additional independent chains to fill ready list */
        int chain1 = int_chain(t1, t2, t3);
        int chain2 = int_chain(t4, arr[i], arr[i + 1]);
        double fp_chain = compute_pressure(d1, d2) + compute_pressure(d3, d4);
        
        sink = chain1 + chain2;
        acc += fp_chain;
    }
    
    /* Conditional update at end of block - may trigger state saving */
    if (sum > 1000) {
        arr[0] = sum % 256;
        darr[0] = acc / n;
    }
}

/* Function with speculative scheduling opportunities */
void speculative_block(int *data, int size) {
    int i, j;
    int result = 0;
    
    /* Inner loop with small iteration count - candidate for software pipelining */
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Complex conditional chain - creates scheduling barriers */
        if (val > 0) {
            /* Multiple dependent operations in conditional path */
            int a = val * 2 + 1;
            int b = a << 3;
            int c = b - val;
            int d = c ^ a;
            
            /* Function call with side effects (inline asm) */
            asm volatile("" : "+r" (d) : : "memory");
            
            result += d;
            
            /* Floating point in conditional path */
            double fp_val = (double)val * 0.5;
            fp_val = fp_val * fp_val - sqrt(fp_val);
            sink = (int)fp_val;
        } else if (val < 0) {
            /* Different execution path */
            int a = val * 3 - 2;
            int b = a >> 1;
            int c = b | 0xFF;
            result -= c;
        } else {
            /* Third path with memory operations */
            for (j = 0; j < 4; j++) {
                data[j] = data[j] + i;
            }
        }
        
        /* Additional computation to widen the block */
        data[i] = result ^ val;
    }
}

/* Function using GCC vector extensions to create parallel operations */
void vectorized_block(int *restrict a, int *restrict b, int *restrict c, int n) {
    /* Define vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    int i;
    
    /* Process in chunks of 4 */
    for (i = 0; i < n - 3; i += 4) {
        /* Load vectors */
        v4si va = {a[i], a[i+1], a[i+2], a[i+3]};
        v4si vb = {b[i], b[i+1], b[i+2], b[i+3]};
        
        /* Multiple vector operations - creates many parallel instructions */
        v4si vc1 = va + vb;
        v4si vc2 = va - vb;
        v4si vc3 = va * vb;
        v4si vc4 = (va << 2) | vb;
        
        /* Mixed with scalar operations */
        int s1 = a[i] * b[i];
        int s2 = a[i+1] + b[i+1];
        int s3 = a[i+2] - b[i+2];
        int s4 = a[i+3] ^ b[i+3];
        
        /* Store results - creates memory dependencies */
        c[i] = vc1[0] + s1;
        c[i+1] = vc2[1] + s2;
        c[i+2] = vc3[2] + s3;
        c[i+3] = vc4[3] + s4;
        
        /* Additional FP vector operations */
        v4sf vfa = {(float)a[i], (float)a[i+1], (float)a[i+2], (float)a[i+3]};
        v4sf vfb = {(float)b[i], (float)b[i+1], (float)b[i+2], (float)b[i+3]};
        v4sf vfc = vfa * vfb + vfa / (vfb + 1.0f);
        
        /* Use results to prevent optimization */
        sink = vfc[0] + vfc[1] + vfc[2] + vfc[3];
    }
}

/* Function with switch statement for complex control flow */
void switch_block(int mode, int *data, int n) {
    int i;
    int result = 0;
    
    for (i = 0; i < n; i++) {
        /* Switch creates complex control flow requiring state tracking */
        switch (mode) {
            case 0:
                /* Integer intensive path */
                data[i] = data[i] * 3 + 7;
                result += data[i] << 1;
                break;
            case 1:
                /* FP intensive path */
                {
                    double val = (double)data[i];
                    val = val * 1.5 - sqrt(fabs(val));
                    data[i] = (int)val;
                    result += (int)(val * 100);
                }
                break;
            case 2:
                /* Memory intensive path */
                if (i > 0) {
                    data[i] = data[i] + data[i-1] - data[(i+1)%n];
                }
                result ^= data[i];
                break;
            case 3:
                /* Mixed operations */
                {
                    int temp = data[i] * data[i];
                    double ftemp = sin((double)temp);
                    data[i] = (int)(ftemp * 1000);
                    result = result * 2 + temp;
                }
                break;
            default:
                /* Default path with inline asm */
                asm volatile (
                    "mov %0, %0\n"
                    "add $1, %0\n"
                    : "+r" (data[i])
                    :
                    : "cc"
                );
                result -= data[i];
        }
        
        /* Additional computation to create scheduling pressure */
        if (i % 8 == 0) {
            data[i] = int_chain(data[i], result, i);
        }
    }
    
    sink = result;
}

/* Function with very wide basic block via manual unrolling */
void wide_block(float *restrict a, float *restrict b, float *restrict c, int n) {
    /* Manual unrolling creates a very wide basic block */
    int i;
    for (i = 0; i < n; i += 8) {
        /* 8 independent chains of dependent operations */
        float a0 = a[i] * 2.0f + 1.0f;
        float b0 = b[i] * 3.0f - 2.0f;
        c[i] = a0 * b0 - sqrtf(fabsf(a0 - b0));
        
        float a1 = a[i+1] * 1.5f + 0.5f;
        float b1 = b[i+1] * 2.5f - 1.5f;
        c[i+1] = a1 / (b1 + 1.0f) + cosf(a1);
        
        float a2 = a[i+2] * 0.7f + 2.3f;
        float b2 = b[i+2] * 1.7f - 0.3f;
        c[i+2] = sinf(a2) * cosf(b2) + tanf(a2 - b2);
        
        float a3 = a[i+3] * 3.7f + 1.2f;
        float b3 = b[i+3] * 2.2f - 0.8f;
        c[i+3] = expf(fabsf(a3 - b3)) * logf(fabsf(a3) + 1.0f);
        
        float a4 = a[i+4] * 1.1f + 3.3f;
        float b4 = b[i+4] * 4.4f - 2.2f;
        c[i+4] = powf(a4, 1.5f) + powf(b4, 0.5f);
        
        float a5 = a[i+5] * 2.8f + 0.7f;
        float b5 = b[i+5] * 1.9f - 0.4f;
        c[i+5] = a5 * b5 * (a5 + b5) / (a5 - b5 + 5.0f);
        
        float a6 = a[i+6] * 0.9f + 4.1f;
        float b6 = b[i+6] * 3.3f - 1.1f;
        c[i+6] = sqrtf(a6 * a6 + b6 * b6) - hypotf(a6, b6);
        
        float a7 = a[i+7] * 1.8f + 2.2f;
        float b7 = b[i+7] * 2.7f - 0.9f;
        c[i+7] = (a7 + b7) * (a7 - b7) / (a7 * b7 + 1.0f);
        
        /* Cross-dependent operations */
        c[i] += c[i+1] * 0.1f;
        c[i+1] += c[i+2] * 0.2f;
        c[i+2] += c[i+3] * 0.3f;
        c[i+3] += c[i+4] * 0.4f;
        c[i+4] += c[i+5] * 0.5f;
        c[i+5] += c[i+6] * 0.6f;
        c[i+6] += c[i+7] * 0.7f;
        c[i+7] += c[i] * 0.8f;
    }
}

/* Main driver that calls all test functions */
int main() {
    const int SIZE = 256;
    const int ITER = 10;
    
    /* Allocate test arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    double *darr = malloc(SIZE * sizeof(double));
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    float *farr3 = malloc(SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = 0;
        darr[i] = (double)(rand() % 1000) / 10.0;
        farr1[i] = (float)(rand() % 1000) / 10.0f;
        farr2[i] = (float)(rand() % 1000) / 10.0f;
        farr3[i] = 0.0f;
    }
    
    printf("Starting scheduler coverage test...\n");
    
    /* Call each test function multiple times to ensure scheduling happens */
    for (int iter = 0; iter < ITER; iter++) {
        /* Test 1: Complex block with mixed operations */
        complex_block_1(arr1, darr, SIZE);
        
        /* Test 2: Speculative scheduling block */
        speculative_block(arr2, SIZE);
        
        /* Test 3: Vectorized operations */
        vectorized_block(arr1, arr2, arr3, SIZE);
        
        /* Test 4: Switch-based control flow */
        switch_block(iter % 4, arr3, SIZE);
        
        /* Test 5: Very wide basic block */
        wide_block(farr1, farr2, farr3, SIZE);
        
        /* Mix up data to create different scheduling patterns */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = (arr1[i] + arr3[i]) % 1000;
            arr2[i] = (arr2[i] ^ arr3[i]) % 1000;
            farr1[i] = fabsf(sinf(farr3[i] * 0.1f) * 100.0f);
        }
    }
    
    /* Compute checksum to verify computations weren't optimized away */
    int int_sum = 0;
    double double_sum = 0.0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < SIZE; i++) {
        int_sum += arr1[i] + arr2[i] + arr3[i];
        double_sum += darr[i];
        float_sum += farr1[i] + farr2[i] + farr3[i];
    }
    
    printf("Checksums:\n");
    printf("  Integer sum: %d\n", int_sum);
    printf("  Double sum: %f\n", double_sum);
    printf("  Float sum: %f\n", float_sum);
    printf("  Sink value: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(darr);
    free(farr1);
    free(farr2);
    free(farr3);
    
    printf("Test completed.\n");
    
    return 0;
}
