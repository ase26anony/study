/* test_sched_context.c - Complex scheduling test for GCC Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline)) static inline

/* Vector types for parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile function to prevent optimization */
volatile int global_counter = 0;

/* Function with side effects - creates scheduling barriers */
ALWAYS_INLINE int side_effect_func(int x) {
    global_counter += x;
    return x + 1;
}

/* Complex integer computation chain */
ALWAYS_INLINE int int_chain_compute(int a, int b, int c, int d, int e) {
    int t1 = a * b + c;
    int t2 = t1 / (d + 1);
    int t3 = t2 ^ e;
    int t4 = t3 << 3;
    int t5 = t4 - b;
    int t6 = t5 * a;
    int t7 = t6 | c;
    int t8 = t7 & 0x7FFFFFFF;
    return t8;
}

/* Floating-point computation chain */
ALWAYS_INLINE float fp_chain_compute(float a, float b, float c, float d, float e) {
    float t1 = a * b + c;
    float t2 = t1 / (d + 1.0f);
    float t3 = sinf(t2) * e;
    float t4 = t3 - b;
    float t5 = t4 * a;
    float t6 = cosf(t5) + c;
    float t7 = t6 * 0.5f;
    return t7;
}

/* Memory-intensive computation with potential aliasing */
ALWAYS_INLINE void memory_ops(int* arr1, int* arr2, int* arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = arr2[i] * arr3[i];
        arr2[i] = arr1[i] + arr3[i];
        arr3[i] = arr2[i] - arr1[i];
    }
}

/* Function with speculative scheduling opportunities */
int speculative_compute(int* data, int n, int threshold) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Complex dependency chain */
        int val = data[i];
        val = val * 3 + 7;
        val = val >> 1;
        val = val ^ 0x55AA55AA;
        
        /* Conditional that might be speculatively scheduled */
        if (val > threshold) {
            sum += val * 2;
            /* More operations in taken branch */
            sum = sum - (val >> 4);
            sum = sum | 0x0000FFFF;
        } else {
            sum += val / 2;
            /* Different operations in else branch */
            sum = sum & 0xFFFF0000;
            sum = sum + 256;
        }
        
        /* Additional operations after conditional */
        sum = sum * 1.5;  /* Mixed type operation */
        sum = abs(sum);   /* Function call with side effects */
    }
    return sum;
}

/* Wide basic block with unrolled loop */
void wide_basic_block(int* input, int* output, int size) {
    /* Unroll manually to create wide block */
    for (int i = 0; i < size; i += 8) {
        /* Independent computation chains - fills ready list */
        int a0 = input[i] * 3 + 7;
        int b0 = input[i+1] * 5 - 3;
        int c0 = input[i+2] * 2 + 1;
        int d0 = input[i+3] * 7 - 5;
        int e0 = input[i+4] * 11 + 13;
        int f0 = input[i+5] * 13 - 11;
        int g0 = input[i+6] * 17 + 19;
        int h0 = input[i+7] * 19 - 17;
        
        /* More dependent operations */
        a0 = a0 ^ b0;
        b0 = b0 | c0;
        c0 = c0 & d0;
        d0 = d0 + e0;
        e0 = e0 - f0;
        f0 = f0 * g0;
        g0 = g0 / (h0 + 1);
        h0 = h0 << 2;
        
        /* Cross-chain dependencies */
        int x0 = a0 + b0 + c0 + d0;
        int y0 = e0 + f0 + g0 + h0;
        int z0 = x0 * y0 - (a0 * h0);
        
        /* Store results */
        output[i] = a0 + z0;
        output[i+1] = b0 + z0;
        output[i+2] = c0 + z0;
        output[i+3] = d0 + z0;
        output[i+4] = e0 + z0;
        output[i+5] = f0 + z0;
        output[i+6] = g0 + z0;
        output[i+7] = h0 + z0;
    }
}

/* Mixed integer/FP operations with SIMD-like patterns */
void mixed_operations(float* farr, int* iarr, double* darr, int n) {
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int ival = iarr[i];
        ival = ival * 3 + 7;
        ival = ival >> 1;
        ival = ival ^ 0x12345678;
        
        /* Floating-point operations */
        float fval = farr[i];
        fval = fval * 1.5f + 2.3f;
        fval = sinf(fval) * cosf(fval);
        
        /* Double precision operations */
        double dval = darr[i];
        dval = dval * 2.5 + 3.7;
        dval = sqrt(fabs(dval));
        
        /* Mixed type computation */
        farr[i] = fval + (float)ival + (float)dval;
        iarr[i] = ival + (int)fval + (int)dval;
        darr[i] = dval + (double)ival + (double)fval;
        
        /* Function call with side effect */
        side_effect_func(ival & 0xFF);
    }
}

/* Complex function with switch statement for state saving */
int switch_based_compute(int mode, int x, int y, int z) {
    int result = 0;
    
    switch (mode % 5) {
        case 0:
            result = x * y + z;
            result = result << 3;
            result = result | 0xAA;
            break;
        case 1:
            result = x + y * z;
            result = result >> 2;
            result = result ^ 0x55;
            break;
        case 2:
            result = x * z - y;
            result = result * 3;
            result = result & 0xFF;
            break;
        case 3:
            result = y * z - x;
            result = result / 2;
            result = result + 100;
            break;
        case 4:
            result = x + y + z;
            result = result * result;
            result = result % 1000;
            break;
    }
    
    /* Additional operations after switch */
    result = result * 2;
    result = abs(result);
    result = result + global_counter;
    
    return result;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    const int ITER = 100;
    
    /* Allocate arrays with different alignments */
    int* arr1 = (int*)aligned_alloc(64, SIZE * sizeof(int));
    int* arr2 = (int*)aligned_alloc(64, SIZE * sizeof(int));
    int* arr3 = (int*)aligned_alloc(64, SIZE * sizeof(int));
    float* farr = (float*)aligned_alloc(64, SIZE * sizeof(float));
    double* darr = (double*)aligned_alloc(64, SIZE * sizeof(double));
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        farr[i] = (float)rand() / RAND_MAX * 100.0f;
        darr[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    int total_sum = 0;
    
    /* Run multiple test functions to exercise different scheduling scenarios */
    for (int iter = 0; iter < ITER; iter++) {
        /* Test 1: Wide basic block */
        wide_basic_block(arr1, arr2, SIZE);
        
        /* Test 2: Mixed operations */
        mixed_operations(farr, arr1, darr, SIZE);
        
        /* Test 3: Speculative computation */
        total_sum += speculative_compute(arr1, SIZE, 500);
        
        /* Test 4: Memory operations with aliasing */
        memory_ops(arr1, arr2, arr3, SIZE);
        
        /* Test 5: Switch-based computation */
        for (int i = 0; i < SIZE; i++) {
            total_sum += switch_based_compute(i, arr1[i], arr2[i], arr3[i]);
        }
        
        /* Test 6: Complex chains */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = int_chain_compute(arr1[i], arr2[i], arr3[i], 
                                       (i % 10) + 1, total_sum & 0xFF);
            farr[i] = fp_chain_compute(farr[i], (float)arr2[i], 
                                      (float)arr3[i], (i % 5) + 1.0f, 
                                      (total_sum & 0xFF) / 255.0f);
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + (int)farr[i] + (int)darr[i];
        checksum = checksum ^ (checksum >> 16);
    }
    
    printf("Total sum: %d, Checksum: %d, Global counter: %d\n", 
           total_sum, checksum, global_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    free(darr);
    
    return 0;
}
