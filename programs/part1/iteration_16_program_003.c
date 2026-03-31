/* Selective Scheduling Stress Test for GCC Coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3", "unroll-loops")))
static float hot_loop_scheduler(float *data, int size) {
    volatile float result = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < size - 1; i++) {
        /* RAW hazard: read after write */
        float temp = data[i] * 2.0f;
        result += temp + data[i + 1];
        
        /* WAR hazard: write after read */
        data[i] = result * 0.5f;
        
        /* WAW hazard: write after write */
        temp = data[i] * 3.0f;
        data[i] = temp * 0.333f;
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_pointer_chaser(int *ptr, int steps) {
    int sum = 0;
    volatile int *current = ptr;
    
    /* Pointer chasing with varying latencies */
    for (int i = 0; i < steps; i++) {
        /* Load with potential cache miss pattern */
        sum += *current;
        
        /* Complex address calculation */
        current = ptr + ((sum * 1103515245 + 12345) & 1023);
        
        /* Assembly with register clobbers */
        asm volatile(
            "addl $1, %0\n\t"
            : "+r" (sum)
            : 
            : "cc"
        );
    }
    
    return sum;
}

__attribute__((optimize("O3")))
static void vectorized_unrolled_loop(double *a, double *b, double *c, int n) {
    int i;
    
    /* SIMD-friendly loop with pragma unroll */
    #pragma GCC unroll 4
    for (i = 0; i < (n & ~3); i += 4) {
        /* Vectorizable operations */
        a[i] = b[i] * c[i] + 1.0;
        a[i+1] = b[i+1] * c[i+1] + 2.0;
        a[i+2] = b[i+2] * c[i+2] + 3.0;
        a[i+3] = b[i+3] * c[i+3] + 4.0;
        
        /* Conditional move mixed with computation */
        double t = (a[i] > 0.0) ? a[i] : -a[i];
        a[i] = t * ((i % 2) ? 0.5 : 2.0);
    }
    
    /* Remainder loop */
    for (; i < n; i++) {
        a[i] = b[i] * c[i];
    }
}

__attribute__((noinline))
static int complex_control_flow(int x, int *counter) {
    int result = 0;
    
    /* Nested control flow with switch */
    switch (x & 0xF) {
        case 0: result = x * 2; break;
        case 1: result = x + (*counter)++; break;
        case 2: result = x >> 1; break;
        case 3: result = x | 0xFF; break;
        case 4: result = x & 0xAA; break;
        case 5: result = x ^ 0x55; break;
        default: 
            /* Multiple early exit points */
            if (x < 0) return -1;
            if (x > 1000) return 1000;
            result = x % 7;
            break;
    }
    
    /* Mixed branching and conditional moves */
    for (int i = 0; i < 8; i++) {
        /* Branch with side effect */
        if (result & (1 << i)) {
            (*counter) += i;
        } else {
            result |= (1 << i);
        }
        
        /* Conditional operator creating phi nodes */
        result = (i % 3 == 0) ? result * 3 : result / 2;
    }
    
    return result;
}

__attribute__((optimize("O3", "tree-vectorize")))
static void mixed_data_types_computation(float *farr, double *darr, 
                                        int32_t *iarr, int64_t *larr, int n) {
    /* Heterogeneous operations challenging scheduler */
    for (int i = 0; i < n; i++) {
        /* Float to int conversion hazards */
        int32_t ifloat = (int32_t)farr[i];
        
        /* Mixed precision computations */
        darr[i] = (double)farr[i] * 1.5;
        
        /* Integer with varying latencies */
        iarr[i] = ifloat * 1103515245 + 12345;
        
        /* 64-bit operations */
        larr[i] = (int64_t)iarr[i] * iarr[i];
        
        /* Memory barrier splitting scheduling regions */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Additional dependency chain */
        farr[i] = (float)darr[i] * 0.5f + (float)iarr[i];
    }
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for vectorization */
    float *fdata = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *ddata = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int32_t *idata = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int64_t *ldata = (int64_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    int *intdata = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fdata[i] = (float)(i % 100) * 0.1f;
        ddata[i] = (double)(i % 200) * 0.05;
        idata[i] = i;
        ldata[i] = i * 2;
        intdata[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    volatile float acc_float = 0.0f;
    volatile double acc_double = 0.0;
    volatile int acc_int = 0;
    volatile int counter = 0;
    
    /* Execute multiple test patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Hot loop with mixed hazards */
        acc_float += hot_loop_scheduler(fdata, ARRAY_SIZE);
        
        /* Test 2: Cold pointer chasing */
        acc_int += cold_pointer_chaser(intdata, 500);
        
        /* Test 3: Vectorized and unrolled loops */
        vectorized_unrolled_loop(ddata, ddata + 128, ddata + 256, 512);
        acc_double += ddata[iter % 512];
        
        /* Test 4: Complex control flow */
        acc_int += complex_control_flow(iter, &counter);
        
        /* Test 5: Mixed data type computations */
        mixed_data_types_computation(fdata, ddata, idata, ldata, ARRAY_SIZE/2);
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r" (acc_float), "+r" (acc_double), "+r" (acc_int));
    }
    
    /* Use results to prevent optimization */
    printf("Results: float=%f double=%f int=%d counter=%d\n", 
           acc_float, acc_double, acc_int, counter);
    
    /* Cleanup */
    free(fdata);
    free(ddata);
    free(idata);
    free(ldata);
    free(intdata);
    
    return 0;
}
