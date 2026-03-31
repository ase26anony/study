#include <stdio.h>
#include <stdlib.h>
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
        /* RAW hazard: read a[i], write to temp */
        float temp = a[i] * 2.5f;
        
        /* WAR hazard: read temp, write to b[i] */
        b[i] = temp + 1.0f;
        
        /* WAW hazard: multiple writes to sum */
        sum += b[i];
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
        
        /* Pointer chasing pattern */
        c[i] = b[i] * temp;
        
        /* Another memory operation with dependency */
        a[i] = c[i] / 3.14f;
    }
    
    return sum;
}

/* Cold function with different scheduling needs */
__attribute__((cold, noinline, optimize("O3")))
static int cold_function(int* restrict arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                /* Fall through */
            case 1:
                result -= arr[i];
                break;
            case 2:
                result ^= arr[i];
                /* Conditional move */
                result = (result > 1000) ? 1000 : result;
                break;
            case 3:
                /* Early exit condition */
                if (result < 0) continue;
                result *= 3;
                break;
            case 4:
                result |= arr[i];
                break;
            case 5:
                /* Nested if-else */
                if (arr[i] % 2 == 0) {
                    result += arr[i] >> 1;
                } else {
                    result += arr[i] << 1;
                }
                break;
            default:
                result = result * 2 - arr[i];
        }
        
        /* Assembly with register clobber */
        asm volatile("addl $1, %0" : "+r"(result) : : "cc");
    }
    
    return result;
}

/* Function with vectorization opportunities */
__attribute__((optimize("O3")))
static void vectorized_loop(double* restrict a, double* restrict b, 
                           double* restrict c, int n) {
    /* SIMD-friendly loop */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* Multiple dependent FP operations */
        double t1 = sin(a[i] * 0.1);
        double t2 = cos(b[i] * 0.2);
        
        /* Cross-dependency between arrays */
        c[i] = t1 * t2 + a[i] * b[i];
        
        /* Complex expression with multiple uses */
        a[i] = c[i] * t1 - t2;
        b[i] = t1 + t2 * c[i];
        
        /* Memory barrier splitting scheduling regions */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* Function with mixed data types and operations */
__attribute__((optimize("sched-pressure"), noinline))
static long mixed_operations(short* s_arr, int* i_arr, long* l_arr, int n) {
    long total = 0;
    
    for (int i = 0; i < n; i++) {
        /* Type conversions and mixed operations */
        int temp_int = s_arr[i] * 3;
        long temp_long = i_arr[i] * 5L;
        
        /* Multiple dependent operations */
        temp_int = (temp_int << 2) | (temp_int >> 30);
        temp_long = temp_long + (temp_int * 7L);
        
        /* Store with dependency */
        l_arr[i] = temp_long;
        
        /* Accumulate with complex expression */
        total += temp_long * ((i % 3) + 1);
        
        /* Assembly with specific constraints */
        asm volatile("imulq $3, %0" : "+r"(total) : : "cc");
    }
    
    return total;
}

/* Function with nested loops and complex exit conditions */
__attribute__((hot))
static double nested_loop_pattern(float* matrix, int rows, int cols) {
    double checksum = 0.0;
    
    for (int i = 0; i < rows; i++) {
        /* Multiple early exit points */
        if (i % 13 == 0) continue;
        
        float row_sum = 0.0f;
        for (int j = 0; j < cols; j++) {
            /* Complex addressing */
            float val = matrix[i * cols + j];
            
            /* Conditional operations */
            if (val > 0.5f) {
                row_sum += val * 1.5f;
            } else if (val < -0.5f) {
                row_sum -= val * 0.5f;
            } else {
                row_sum += val;
            }
            
            /* Early exit from inner loop */
            if (row_sum > 1000.0f) break;
        }
        
        checksum += row_sum;
        
        /* Another early exit */
        if (checksum > 10000.0) return checksum;
    }
    
    return checksum;
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* farr1 = aligned_alloc(64, SIZE * sizeof(float));
    float* farr2 = aligned_alloc(64, SIZE * sizeof(float));
    float* farr3 = aligned_alloc(64, SIZE * sizeof(float));
    double* darr1 = aligned_alloc(64, SIZE * sizeof(double));
    double* darr2 = aligned_alloc(64, SIZE * sizeof(double));
    double* darr3 = aligned_alloc(64, SIZE * sizeof(double));
    int* iarr = aligned_alloc(64, SIZE * sizeof(int));
    short* sarr = aligned_alloc(64, SIZE * sizeof(short));
    long* larr = aligned_alloc(64, SIZE * sizeof(long));
    float* matrix = aligned_alloc(64, 64 * 64 * sizeof(float));
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
        darr1[i] = (double)rand() / RAND_MAX;
        darr2[i] = (double)rand() / RAND_MAX;
        iarr[i] = rand() % 1000;
        sarr[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = (float)rand() / RAND_MAX - 0.5f;
    }
    
    /* Accumulator for results to prevent dead code elimination */
    volatile float f_acc = 0.0f;
    volatile double d_acc = 0.0;
    volatile long l_acc = 0L;
    volatile int i_acc = 0;
    
    /* Run multiple iterations to stress the scheduler */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call all test functions in sequence */
        f_acc += hot_function(farr1, farr2, farr3, SIZE);
        
        i_acc += cold_function(iarr, SIZE);
        
        vectorized_loop(darr1, darr2, darr3, SIZE);
        d_acc += darr3[SIZE / 2];
        
        l_acc += mixed_operations(sarr, iarr, larr, SIZE);
        
        d_acc += nested_loop_pattern(matrix, 64, 64);
        
        /* Modify inputs slightly for next iteration */
        for (int i = 0; i < SIZE; i++) {
            farr1[i] += 0.001f;
            darr1[i] += 0.001;
            iarr[i] = (iarr[i] + 1) % 1000;
        }
    }
    
    /* Print results to ensure all computations are used */
    printf("Results: f_acc=%f, d_acc=%f, l_acc=%ld, i_acc=%d\n", 
           f_acc, d_acc, l_acc, i_acc);
    
    /* Cleanup */
    free(farr1);
    free(farr2);
    free(farr3);
    free(darr1);
    free(darr2);
    free(darr3);
    free(iarr);
    free(sarr);
    free(larr);
    free(matrix);
    
    return 0;
}
