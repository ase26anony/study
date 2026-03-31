#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function with complex scheduling patterns */
__attribute__((hot, optimize("O3", "unroll-loops")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: reusing temp */
        b[i] = temp + 1.0f;
        
        /* WAW hazard: multiple writes to sum */
        sum += b[i];
        
        /* Complex dependency chain */
        c[i] = (temp * b[i]) / (sum + 1.0f);
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_function(double* restrict arr, int* restrict indices, int n) {
    double result = 0.0;
    volatile double* vptr = arr; /* Prevent optimizations */
    
    /* Pointer chasing with mixed access patterns */
    for (int i = 0; i < n; i++) {
        int idx = indices[i] % n;
        
        /* Complex control flow */
        switch (idx % 7) {
            case 0:
                result += vptr[idx] * 2.0;
                break;
            case 1:
                result -= vptr[idx] / 3.0;
                break;
            case 2:
                result *= 1.1 + vptr[idx];
                break;
            case 3:
                /* Conditional move pattern */
                result = (idx > n/2) ? result + vptr[idx] : result - vptr[idx];
                break;
            case 4:
                result = sqrt(fabs(result + vptr[idx]));
                break;
            case 5:
                /* Assembly with register clobber */
                asm volatile("" : "+r"(result) : : "r0", "r1", "r2", "r3");
                result += vptr[idx];
                break;
            default:
                result = result * 0.99 + vptr[idx];
        }
        
        /* Early exit condition */
        if (result > 1e6) {
            break;
        }
        
        /* Continue condition */
        if (idx % 3 == 0) {
            continue;
        }
        
        /* Additional computation */
        result = fmod(result, 1000.0);
    }
    
    return result;
}

/* Function with SIMD-friendly patterns */
__attribute__((optimize("O3", "tree-vectorize")))
static void vectorized_loop(int* restrict src, int* restrict dst, int n) {
    /* Compile-time known size helps vectorization */
    int local_buf[ARRAY_SIZE];
    
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* Multiple dependency chains */
        int val1 = src[i] * 3;
        int val2 = src[(i + 1) % n] + 7;
        int val3 = src[(i + 2) % n] - 5;
        
        /* Cross-iteration dependency */
        dst[i] = val1 + val2 + val3 + (i > 0 ? dst[i-1] : 0);
        
        /* Independent computation */
        local_buf[i] = src[i] * src[i];
        
        /* Memory barrier splitting scheduling regions */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Use results to prevent elimination */
    for (int i = 0; i < n; i++) {
        dst[i] += local_buf[i] % 256;
    }
}

/* Function with nested loops and mixed operations */
__attribute__((noinline))
static double nested_loop_scheduler_test(int size) {
    double matrix[64][64];
    double sum = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (i * 1.5 + j * 0.7) / (1.0 + i + j);
        }
    }
    
    /* Complex nested loop with dependencies */
    for (int iter = 0; iter < size; iter++) {
        for (int i = 1; i < 63; i++) {
            for (int j = 1; j < 63; j++) {
                /* Stencil computation with multiple dependencies */
                double north = matrix[i-1][j];
                double south = matrix[i+1][j];
                double east = matrix[i][j+1];
                double west = matrix[i][j-1];
                
                /* Mixed float/int operations */
                double avg = (north + south + east + west) / 4.0;
                double diff = avg - matrix[i][j];
                
                /* Conditional update */
                matrix[i][j] = (fabs(diff) > 0.001) ? 
                    matrix[i][j] + diff * 0.5 : 
                    matrix[i][j];
                
                /* Accumulate with varying patterns */
                sum += matrix[i][j] * ((i * j) % 7);
                
                /* Computed goto-like pattern using switch */
                switch ((i + j + iter) % 5) {
                    case 0: sum *= 0.999; break;
                    case 1: sum += 0.001; break;
                    case 2: sum = fabs(sum); break;
                    case 3: sum = -sum; break;
                    case 4: /* fall through */
                    default: sum = sum / 1.0001;
                }
            }
        }
        
        /* Scheduling barrier every few iterations */
        if (iter % 4 == 0) {
            asm volatile("" : "+m"(matrix) : : "memory");
        }
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* fa = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* fb = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* fc = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    double* da = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* indices = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* src_int = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* dst_int = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)(i * 0.1);
        fb[i] = (float)(i * 0.2);
        fc[i] = (float)(i * 0.3);
        da[i] = (double)(i * 0.05);
        indices[i] = (i * 13 + 7) % ARRAY_SIZE;
        src_int[i] = i * 3 - ARRAY_SIZE/2;
        dst_int[i] = 0;
    }
    
    double total_result = 0.0;
    
    /* Run multiple test functions to exercise different scheduling scenarios */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Hot path with tight loops */
        float hot_result = hot_function(fa, fb, fc, ARRAY_SIZE);
        total_result += hot_result;
        
        /* Cold path with complex control flow */
        double cold_result = cold_function(da, indices, ARRAY_SIZE);
        total_result += cold_result;
        
        /* Vectorized loop test */
        vectorized_loop(src_int, dst_int, ARRAY_SIZE);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            total_result += dst_int[i] * 0.001;
        }
        
        /* Nested loop test */
        double nested_result = nested_loop_scheduler_test(10);
        total_result += nested_result;
        
        /* Modify arrays to create different patterns */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            fa[i] = fb[i] * 0.9f + fc[i] * 0.1f;
            da[i] = sin(da[i] * 0.01);
            src_int[i] = (src_int[i] * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", total_result);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(da);
    free(indices);
    free(src_int);
    free(dst_int);
    
    return 0;
}
