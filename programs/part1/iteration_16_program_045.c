#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function with complex data dependencies and scheduling challenges */
__attribute__((hot, optimize("O3")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed RAW, WAR, WAW hazards */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        float t1 = a[i] * 2.0f;      /* WAW hazard potential */
        float t2 = b[i] + t1;        /* RAW on t1 */
        a[i] = t2 * 0.5f;            /* WAR on a[i], WAW on a[i] */
        c[i] = a[i] + b[i];          /* RAW on a[i] and b[i] */
        sum += c[i];                 /* RAW on c[i] */
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_function(double* restrict arr, int* restrict indices, int n) {
    double result = 0.0;
    
    /* Pointer chasing with complex addressing */
    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        double val = arr[idx & (n-1)];  /* Pointer chase */
        
        /* Complex control flow */
        switch (idx % 7) {
            case 0: val *= 1.1; break;
            case 1: val += 2.3; break;
            case 2: val = sqrt(val); break;
            case 3: val = val * val; break;
            case 4: val = 1.0 / (val + 1.0); break;
            case 5: val = sin(val); break;
            default: val = cos(val); break;
        }
        
        /* Conditional move mixed with computation */
        result = (val > 0.5) ? (result + val) : (result - val);
        
        /* Assembly with register clobber */
        asm volatile("" : "+r"(result) :: "r0", "r1", "cc");
    }
    
    return result;
}

/* Function with SIMD-friendly operations */
__attribute__((optimize("O3"), noinline))
static void vectorized_loop(float* restrict out, 
                           const float* restrict in1, 
                           const float* restrict in2, 
                           int size) {
    /* SIMD-friendly loop with multiple dependencies */
    #pragma GCC unroll 8
    for (int i = 0; i < size; i++) {
        /* Multiple dependent FP operations */
        float x = in1[i];
        float y = in2[i];
        
        /* Create long dependency chain */
        float t1 = x * y;
        float t2 = t1 + x;
        float t3 = t2 - y;
        float t4 = t3 * 1.5f;
        float t5 = t4 / (fabsf(x) + 1.0f);
        float t6 = t5 + sinf(y);
        
        out[i] = t6;
        
        /* Early exit condition */
        if (out[i] > 1000.0f) {
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            break;
        }
    }
}

/* Function with mixed integer/FP operations */
__attribute__((hot, optimize("O3")))
static int mixed_operations(int* restrict int_arr, 
                           float* restrict float_arr, 
                           int n) {
    int int_sum = 0;
    float float_sum = 0.0f;
    
    /* Loop with mixed operation types */
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int_arr[i] = (int_arr[i] * 3 + 7) & 0xFF;
        
        /* Floating point operations */
        float_arr[i] = float_arr[i] * 2.0f - 1.0f;
        
        /* Mixed type computation */
        float_sum += float_arr[i] * int_arr[i];
        int_sum += int_arr[i];
        
        /* Continue condition with complex logic */
        if ((i % 3) == 0) {
            float_sum *= 0.99f;
            continue;
        }
        
        if ((i % 7) == 0) {
            int_sum >>= 1;
        }
    }
    
    /* Final mixed computation */
    return int_sum + (int)float_sum;
}

/* Function with computed goto for complex control flow */
__attribute__((noinline, optimize("O2")))
static double computed_goto_pattern(double x, int mode) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, 
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    double result = x;
    
    if (mode >= 0 && mode < 8) {
        goto *jump_table[mode];
    }
    
    goto default_case;
    
case_0:
    result = sin(x) * cos(x);
    goto end;
case_1:
    result = x * x + 2 * x + 1;
    goto end;
case_2:
    result = sqrt(fabs(x));
    goto end;
case_3:
    result = 1.0 / (x + 1.0);
    goto end;
case_4:
    result = exp(x * 0.1);
    goto end;
case_5:
    result = log(fabs(x) + 1.0);
    goto end;
case_6:
    result = tan(x * 0.5);
    goto end;
case_7:
    result = x * (1.0 - x);
    goto end;
default_case:
    result = 0.0;
    goto end;
    
end:
    /* Scheduling barrier */
    asm volatile("" : "+r"(result) :: "memory");
    return result;
}

/* Main test driver */
int main(void) {
    /* Allocate and initialize arrays */
    float* a = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* b = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* c = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* d_arr = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* indices = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* int_arr = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float* float_arr = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i * 3) % 100) * 0.2f;
        c[i] = 0.0f;
        d_arr[i] = (double)(i % 200) * 0.05;
        indices[i] = (i * 7) % ARRAY_SIZE;
        int_arr[i] = i;
        float_arr[i] = (float)i * 0.3f;
    }
    
    float total_sum = 0.0f;
    double total_double = 0.0;
    int total_int = 0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with complex dependencies */
        total_sum += hot_function(a, b, c, ARRAY_SIZE);
        
        /* Call cold function with pointer chasing */
        total_double += cold_function(d_arr, indices, ARRAY_SIZE / 4);
        
        /* Vectorized operations */
        vectorized_loop(a, b, c, ARRAY_SIZE);
        
        /* Mixed operations */
        total_int += mixed_operations(int_arr, float_arr, ARRAY_SIZE);
        
        /* Computed goto pattern */
        total_double += computed_goto_pattern(total_double, iter % 9);
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            a[i] += 0.01f;
            b[i] -= 0.005f;
            indices[i] = (indices[i] + 1) % ARRAY_SIZE;
        }
    }
    
    /* Print results to prevent optimization */
    printf("Results: sum=%f, double=%f, int=%d\n", 
           total_sum, total_double, total_int);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d_arr);
    free(indices);
    free(int_arr);
    free(float_arr);
    
    return 0;
}
