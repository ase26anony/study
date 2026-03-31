#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling pressure */
__attribute__((hot, optimize("O3", "sched-pressure")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer and floating point operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        b[i] = temp;
        
        /* WAR hazard: temp reused */
        temp = b[i] + 1.0f;
        
        /* WAW hazard: c written multiple times */
        c[i] = temp * 3.0f;
        c[i] = c[i] / 2.0f;  /* Overwrite previous value */
        
        /* Pointer chasing pattern */
        float* ptr = &c[i];
        *ptr = *ptr + sinf(*ptr);
        
        sum += c[i];
    }
    
    /* Memory barrier forcing scheduler decisions */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_function(double* restrict arr, int n) {
    double result = 0.0;
    
    /* Complex control flow with switch statement */
    for (int i = 0; i < n; i++) {
        int selector = i % 7;
        
        switch (selector) {
            case 0:
                arr[i] = arr[i] * 1.1;
                break;
            case 1:
                arr[i] = arr[i] + 2.5;
                /* Fall through */
            case 2:
                arr[i] = arr[i] - 1.0;
                break;
            case 3:
                arr[i] = sqrt(arr[i] + 1.0);
                break;
            case 4:
                /* Conditional move pattern */
                arr[i] = (arr[i] > 0) ? arr[i] * 2.0 : arr[i] / 2.0;
                break;
            case 5:
                arr[i] = arr[i] * arr[i];
                break;
            default:
                arr[i] = 1.0 / (arr[i] + 0.001);
                break;
        }
        
        /* Early exit condition */
        if (arr[i] > 1000.0) {
            result += 1000.0;
            continue;
        }
        
        result += arr[i];
    }
    
    return result;
}

/* SIMD-friendly function with unrolling directives */
__attribute__((optimize("O3")))
static void vectorized_loop(int* restrict src, int* restrict dst, int n) {
    int i;
    
    /* Manual unrolling with pragma hint */
    #pragma GCC unroll 4
    for (i = 0; i < (n & ~3); i += 4) {
        /* Independent operations for vectorization */
        dst[i] = src[i] * 2;
        dst[i+1] = src[i+1] + 5;
        dst[i+2] = src[i+2] - 3;
        dst[i+3] = src[i+3] / 2;
        
        /* Cross-iteration dependency */
        if (i > 0) {
            dst[i] += dst[i-1];
        }
    }
    
    /* Remainder loop */
    for (; i < n; i++) {
        dst[i] = src[i] * src[i];
    }
    
    /* Assembly with register clobbering */
    asm volatile(
        "mov $0, %%eax\n\t"
        "cpuid\n\t"
        : 
        : 
        : "%eax", "%ebx", "%ecx", "%edx", "memory"
    );
}

/* Function with mixed data types and pointer aliasing */
__attribute__((noinline))
static long mixed_operations(short* s_arr, int* i_arr, long* l_arr, int n) {
    long total = 0;
    
    /* Nested loops with varying dependencies */
    for (int i = 0; i < n; i++) {
        short s_val = s_arr[i];
        
        for (int j = 0; j < 8; j++) {
            /* Mixed-type operations */
            int temp = s_val * j;
            
            /* Pointer chasing with different types */
            i_arr[i] += temp;
            
            /* Floating point in integer loop */
            float f_temp = (float)temp / (j + 1);
            i_arr[i] += (int)f_temp;
            
            /* Memory barrier splitting scheduling regions */
            if (j % 3 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* WAW hazard */
        l_arr[i] = i_arr[i] * 2L;
        l_arr[i] = l_arr[i] + s_arr[i];  /* Overwrite */
        
        total += l_arr[i];
        
        /* Multiple exit points */
        if (total > 1000000L) {
            break;
        }
        
        if (i % 50 == 0) {
            continue;  /* Skip occasional iterations */
        }
    }
    
    return total;
}

/* Function with computed goto for complex control flow */
__attribute__((optimize("O2")))
static int computed_goto_pattern(int* arr, int n) {
    static void* jump_table[] = {
        &&add_op, &&sub_op, &&mul_op, &&div_op, &&mod_op
    };
    
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int op = arr[i] % 5;
        
        goto *jump_table[op];
        
    add_op:
        result += arr[i];
        continue;
        
    sub_op:
        result -= arr[i];
        continue;
        
    mul_op:
        result *= (arr[i] | 1);  /* Avoid multiplication by 0 */
        continue;
        
    div_op:
        if (arr[i] != 0) {
            result /= arr[i];
        }
        continue;
        
    mod_op:
        if (arr[i] != 0) {
            result %= arr[i];
        }
        continue;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* f_arr1 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* f_arr2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* f_arr3 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    
    double* d_arr = (double*)aligned_alloc(32, SIZE * sizeof(double));
    
    int* i_arr1 = (int*)aligned_alloc(32, SIZE * sizeof(int));
    int* i_arr2 = (int*)aligned_alloc(32, SIZE * sizeof(int));
    
    short* s_arr = (short*)aligned_alloc(16, SIZE * sizeof(short));
    long* l_arr = (long*)aligned_alloc(32, SIZE * sizeof(long));
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        f_arr1[i] = (float)(i % 100) * 0.1f;
        f_arr2[i] = (float)(i % 50) * 0.2f;
        f_arr3[i] = (float)(i % 25) * 0.3f;
        
        d_arr[i] = (double)(i % 200) * 0.05;
        
        i_arr1[i] = i * 2;
        i_arr2[i] = i * 3;
        
        s_arr[i] = (short)(i % 300);
        l_arr[i] = i * 10L;
    }
    
    float total_float = 0.0f;
    double total_double = 0.0;
    long total_long = 0L;
    int total_int = 0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with mixed operations */
        total_float += hot_function(f_arr1, f_arr2, f_arr3, SIZE);
        
        /* Call cold function with complex control flow */
        total_double += cold_function(d_arr, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(i_arr1, i_arr2, SIZE);
        total_int += i_arr2[SIZE / 2];
        
        /* Mixed type operations */
        total_long += mixed_operations(s_arr, i_arr1, l_arr, SIZE / 2);
        
        /* Computed goto pattern */
        total_int += computed_goto_pattern(i_arr1, SIZE / 4);
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            f_arr1[i] += 0.001f;
            d_arr[i] += 0.0005;
            i_arr1[i] += 1;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("Total float: %f\n", total_float);
    printf("Total double: %f\n", total_double);
    printf("Total long: %ld\n", total_long);
    printf("Total int: %d\n", total_int);
    
    /* Final computation using all results */
    double final_result = (double)total_float + total_double + (double)total_long + (double)total_int;
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(f_arr1);
    free(f_arr2);
    free(f_arr3);
    free(d_arr);
    free(i_arr1);
    free(i_arr2);
    free(s_arr);
    free(l_arr);
    
    return (final_result > 0) ? 0 : 1;
}
