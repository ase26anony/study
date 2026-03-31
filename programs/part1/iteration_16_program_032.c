#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function with complex data dependencies and mixed operations */
__attribute__((hot, optimize("O3"))) 
static float hot_function(float* restrict a, float* restrict b, int* restrict c, int n) {
    float sum = 0.0f;
    
    /* Nested loops with mixed dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: overwriting temp */
        temp = temp + (float)c[i];
        
        /* WAW hazard: multiple writes to b[i] */
        b[i] = temp * 0.5f;
        b[i] = b[i] + sinf(temp);  /* FP operation mixed with integer */
        
        /* Pointer chasing pattern */
        int idx = c[i] & (n-1);
        float* ptr = &b[idx];
        
        /* Memory barrier to force scheduling decisions */
        asm volatile("" ::: "memory");
        
        /* Complex expression with multiple dependencies */
        sum += (*ptr) * a[i] + (float)(c[idx] % 256);
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_function(double* restrict arr, int n) {
    double result = 1.0;
    
    /* Switch statement with sparse cases */
    for (int i = 0; i < n; i++) {
        int val = (int)arr[i] % 7;
        
        switch (val) {
            case 0:
                result *= arr[i] + 1.0;
                /* Inline asm with register clobber */
                asm volatile("" ::: "rax", "rcx");
                break;
            case 3:
                result += cos(arr[i]);
                break;
            case 5:
                result -= sqrt(fabs(arr[i]));
                /* Memory barrier */
                asm volatile("" ::: "memory");
                break;
            default:
                /* Conditional move style operation */
                result = (val > 3) ? result * 0.99 : result * 1.01;
                break;
        }
        
        /* Early exit condition */
        if (result > 1000.0) {
            break;
        }
        
        /* Continue with another operation */
        if (i % 2 == 0) {
            result += (double)i * 0.001;
            continue;
        }
    }
    
    return result;
}

/* Function with vectorization-friendly patterns */
__attribute__((optimize("O3"), noinline))
static void vectorized_loop(float* restrict src, float* restrict dst, int n) {
    /* SIMD-friendly loop with array operations */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* Multiple independent chains for ILP */
        float a = src[i] * 3.14f;
        float b = src[(i + 1) % n] * 2.71f;
        float c = src[(i + 2) % n] * 1.41f;
        
        /* Cross-iteration dependency */
        dst[i] = a + b * c;
        
        /* Integer operations mixed with FP */
        int idx = (i * 13) % n;
        dst[idx] += (float)(i & 0xFF);
        
        /* Scheduling barrier with specific constraints */
        asm volatile("" ::: "xmm0", "xmm1", "memory");
    }
}

/* Function with pointer aliasing challenges */
__attribute__((optimize("O2")))
static int pointer_chasing(int* restrict base, int steps) {
    int* current = base;
    int sum = 0;
    
    /* Pointer chasing with computed goto-like pattern */
    for (int i = 0; i < steps; i++) {
        /* Load with potential cache miss */
        int value = *current;
        
        /* Complex address calculation */
        int offset = (value * 1103515245 + 12345) & (ARRAY_SIZE - 1);
        
        /* Scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* Update pointer */
        current = base + offset;
        
        /* Mixed operations */
        sum += value ^ (i * 0x5A827999);
        sum = (sum << 3) | (sum >> 29);  /* Rotation */
        
        /* Another barrier with register clobber */
        asm volatile("" ::: "rax", "rbx", "rcx");
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* fdata1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    int* idata = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* ddata = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fdata1[i] = (float)(i * 0.1);
        fdata2[i] = (float)(i * 0.2);
        idata[i] = i * 3;
        ddata[i] = (double)(i * 0.05);
    }
    
    float total_sum = 0.0f;
    double total_result = 0.0;
    int total_int = 0;
    
    /* Execute multiple times to ensure coverage */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with complex scheduling */
        total_sum += hot_function(fdata1, fdata2, idata, ARRAY_SIZE);
        
        /* Call cold function */
        total_result += cold_function(ddata, ARRAY_SIZE / 2);
        
        /* Vectorized operations */
        vectorized_loop(fdata1, fdata2, ARRAY_SIZE);
        
        /* Pointer chasing pattern */
        total_int += pointer_chasing(idata, ARRAY_SIZE / 4);
        
        /* Modify data to change patterns */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            fdata1[i] += 0.001f;
            idata[i] ^= iter;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %f %f %d\n", total_sum, total_result, total_int);
    
    /* Cleanup */
    free(fdata1);
    free(fdata2);
    free(idata);
    free(ddata);
    
    return 0;
}
