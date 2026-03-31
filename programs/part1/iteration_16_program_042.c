/* Selective Scheduling Stress Test for GCC sel-sched-dump.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3", "unroll-loops")))
static float hot_loop_scheduler(float *restrict a, float *restrict b, 
                               float *restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read a[i], then write */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: write to a[i] after reading */
        a[i] = temp + b[i];
        
        /* WAW hazard: multiple writes to c[i] */
        c[i] = a[i] + b[i];
        c[i] = c[i] * 0.5f;  // Second write
        
        /* Pointer chasing pattern */
        float *ptr = &c[i];
        *ptr = *ptr + sinf(*ptr);
        
        sum += c[i];
    }
    
    /* Memory barrier forcing scheduler decisions */
    asm volatile("" ::: "memory");
    
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_path_scheduler(int *restrict arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                /* Fall through */
            case 1:
                result += arr[i] >> 1;
                break;
            case 2:
            case 3:
                result += arr[i] & 0xFF;
                /* Conditional move */
                result = (arr[i] > 0) ? result : result - 1;
                break;
            default:
                /* Multiple early exit opportunities */
                if (arr[i] < -1000) return result;
                if (arr[i] > 1000) continue;
                result += arr[i] % 13;
        }
        
        /* Inline asm with register clobber */
        asm volatile("" : "=r"(result) : "0"(result) : "eax");
    }
    
    return result;
}

__attribute__((optimize("O3")))
static void vectorized_scheduler(double *restrict a, double *restrict b,
                                double *restrict c, int n) {
    /* SIMD-friendly loop with unroll pragma */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Mixed precision operations */
        double temp = a[i] * 3.14159;
        
        /* Data-dependent addressing */
        int idx = (int)temp % n;
        idx = (idx < 0) ? 0 : idx;
        
        /* Complex expression with multiple uses */
        b[i] = temp + sin(a[idx]) * cos(b[i]);
        c[i] = b[i] * b[i] - 2.0 * a[i] * c[i];
        
        /* Memory barrier splitting scheduling regions */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static int pointer_chasing_scheduler(int **restrict ptr_array, 
                                    int *restrict data, int n) {
    int sum = 0;
    int *current = data;
    
    /* Pointer chasing with mixed access patterns */
    for (int i = 0; i < n; i++) {
        /* Load-store sequence with varying latencies */
        int value = *current;
        
        /* Arithmetic with dependency chain */
        value = value * 1103515245 + 12345;
        
        /* Store with address calculation */
        *current = value & 0x7FFFFFFF;
        
        /* Update pointer (simulating chase) */
        int next_idx = value % n;
        ptr_array[i] = current;
        current = &data[next_idx];
        
        sum += value;
        
        /* Assembly barrier with specific constraints */
        asm volatile("" : "+r"(sum) : : "ebx", "ecx");
    }
    
    return sum;
}

__attribute__((noinline))
static float nested_loop_scheduler(float *matrix, int rows, int cols) {
    float total = 0.0f;
    
    /* Nested loops with mixed dependencies */
    for (int i = 0; i < rows; i++) {
        float row_sum = 0.0f;
        
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Matrix access with stride */
            float val = matrix[i * cols + j];
            
            /* Mixed operations creating scheduling challenges */
            val = val * val + (float)i / (float)(j + 1);
            
            /* Conditional update */
            row_sum += (val > 0) ? val : -val;
            
            /* Write back with potential WAR hazard */
            matrix[i * cols + j] = val * 0.99f;
        }
        
        /* Dependency across loop iterations */
        total += row_sum * (float)i;
        
        /* Periodic memory barrier */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    /* Allocate arrays with alignment hints */
    float *fa = aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *fb = aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *fc = aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    double *da = aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double *db = aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double *dc = aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    int *idata = aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int **ptr_arr = aligned_alloc(32, ARRAY_SIZE * sizeof(int*));
    float *matrix = aligned_alloc(32, 64 * 64 * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)i * 0.1f;
        fb[i] = (float)(ARRAY_SIZE - i) * 0.2f;
        fc[i] = 0.0f;
        da[i] = (double)i * 0.01;
        db[i] = (double)(i * i) * 0.001;
        dc[i] = 0.0;
        idata[i] = i - ARRAY_SIZE/2;
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = (float)(i % 100) * 0.1f;
    }
    
    float total_sum = 0.0f;
    int int_sum = 0;
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify inputs slightly each iteration */
        fa[iter % ARRAY_SIZE] += 0.5f;
        idata[iter % ARRAY_SIZE] ^= iter;
        
        /* Call all scheduler-stressing functions */
        total_sum += hot_loop_scheduler(fa, fb, fc, ARRAY_SIZE);
        int_sum += cold_path_scheduler(idata, ARRAY_SIZE);
        vectorized_scheduler(da, db, dc, ARRAY_SIZE);
        int_sum += pointer_chasing_scheduler(ptr_arr, idata, ARRAY_SIZE / 4);
        total_sum += nested_loop_scheduler(matrix, 64, 64);
        
        /* Prevent dead code elimination */
        asm volatile("" : "+g"(total_sum), "+g"(int_sum));
    }
    
    /* Use results to prevent optimization */
    printf("Final results: float_sum = %f, int_sum = %d\n", 
           total_sum, int_sum);
    
    /* Cleanup */
    free(fa); free(fb); free(fc);
    free(da); free(db); free(dc);
    free(idata);
    free(ptr_arr);
    free(matrix);
    
    return (total_sum != 0.0f || int_sum != 0) ? 0 : 1;
}
