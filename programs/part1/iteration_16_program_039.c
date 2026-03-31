#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with complex scheduling patterns */
__attribute__((hot, optimize("O3")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read a[i], then write to c[i] */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: read b[i] after potential write */
        c[i] = temp + b[i];
        
        /* WAW hazard: multiple writes to sum */
        sum += c[i];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More complex dependency chain */
        a[i] = sinf(c[i]) * cosf(b[i]);
        
        /* Pointer chasing simulation */
        float* ptr = &c[i];
        *ptr = *ptr * 0.5f + 1.0f;
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_function(double* restrict arr, int n) {
    double result = 0.0;
    int i = 0;
    
    /* Complex control flow with early exits */
    while (i < n) {
        if (arr[i] < 0) {
            /* Early continue */
            i++;
            continue;
        }
        
        /* Conditional move vs branch */
        double val = (arr[i] > 100.0) ? arr[i] * 0.1 : arr[i] * 2.0;
        
        /* Switch statement with sparse cases */
        switch (i % 7) {
            case 0:
                result += val * 1.1;
                break;
            case 1:
                result += val * 1.2;
                /* Fall through */
            case 2:
                result += val * 0.9;
                break;
            case 5:
                result += sqrt(val);
                break;
            default:
                result += val;
        }
        
        /* Assembly with register clobber */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        
        i++;
    }
    
    return result;
}

/* Function with SIMD-friendly patterns */
__attribute__((optimize("O3")))
static void vectorized_loop(int* restrict src, int* restrict dst, int n) {
    /* Compile-time known size helps vectorization */
    int local_n = n < SIZE ? n : SIZE;
    
    #pragma GCC unroll 8
    for (int i = 0; i < local_n; i++) {
        /* SIMD-friendly operations */
        int val = src[i];
        
        /* Mixed operations creating scheduling pressure */
        dst[i] = (val * 3 + 7) & 0xFF;
        dst[i] ^= (val >> 4);
        dst[i] += i * 2;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* More dependencies */
        src[i] = dst[i] * 2 - src[i];
    }
}

/* Function with nested loops and complex dependencies */
__attribute__((noinline))
static float nested_loops(float* matrix, int rows, int cols) {
    float total = 0.0f;
    
    for (int i = 0; i < rows; i++) {
        float row_sum = 0.0f;
        
        /* Inner loop with unrolling directive */
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Multiple dependent operations */
            float elem = matrix[i * cols + j];
            float transformed = elem * elem + 1.0f;
            
            /* Branch with predictable pattern */
            if (j % 3 == 0) {
                transformed = transformed * 0.5f;
            } else if (j % 3 == 1) {
                transformed = transformed * 0.75f;
            }
            
            row_sum += transformed;
            
            /* Write back with WAW hazard potential */
            matrix[i * cols + j] = transformed;
        }
        
        total += row_sum;
        
        /* Scheduling barrier between row processing */
        asm volatile("" ::: "r8", "r9", "r10", "r11", "r12");
    }
    
    return total;
}

/* Function using computed goto (challenges scheduler) */
__attribute__((optimize("O2")))
static int computed_goto_pattern(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int result = x;
    int idx = x & 0x7;
    
    goto *jump_table[idx];
    
case_0:
    result = result * 2 + 1;
    /* Fall through */
case_1:
    result = result ^ 0x55;
    goto end;
case_2:
    result = result >> 1;
    /* Fall through */
case_3:
    result = result * 3;
    goto end;
case_4:
    result = result & 0xF0;
    /* Fall through */
case_5:
    result = result + 0x10;
    goto end;
case_6:
    result = result | 0x01;
    /* Fall through */
case_7:
    result = result - 1;
    goto end;
    
end:
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize data */
    float* fa = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* fb = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* fc = (float*)aligned_alloc(32, SIZE * sizeof(float));
    double* darr = (double*)aligned_alloc(32, SIZE * sizeof(double));
    int* isrc = (int*)aligned_alloc(32, SIZE * sizeof(int));
    int* idst = (int*)aligned_alloc(32, SIZE * sizeof(int));
    float* matrix = (float*)aligned_alloc(32, SIZE * SIZE * sizeof(float));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 50.0f;
        darr[i] = (double)rand() / RAND_MAX * 200.0;
        isrc[i] = rand() % 256;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i * SIZE + j] = (float)rand() / RAND_MAX;
        }
    }
    
    float total_sum = 0.0f;
    double cold_sum = 0.0;
    int int_sum = 0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function (should trigger selective scheduling) */
        total_sum += hot_function(fa, fb, fc, SIZE);
        
        /* Call cold function */
        cold_sum += cold_function(darr, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(isrc, idst, SIZE);
        
        /* Accumulate results to prevent elimination */
        for (int i = 0; i < SIZE; i++) {
            int_sum += idst[i];
        }
        
        /* Nested loops */
        total_sum += nested_loops(matrix, 32, 32);
        
        /* Computed goto pattern */
        int_sum += computed_goto_pattern(iter & 0xFF);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            fa[i] += 0.01f;
            fb[i] *= 0.99f;
            darr[i] += 0.005;
            isrc[i] = (isrc[i] + 1) & 0xFF;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: hot_sum=%.2f, cold_sum=%.2f, int_sum=%d\n", 
           total_sum, cold_sum, int_sum);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(darr);
    free(isrc);
    free(idst);
    free(matrix);
    
    return 0;
}
