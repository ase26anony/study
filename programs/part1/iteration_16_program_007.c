/* Selective Scheduling Stress Test for GCC Coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, noinline, optimize("O3")))
static int hot_function(int *data, float *fdata, int n) {
    int sum = 0;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Complex loop with mixed dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        int temp = data[i];
        data[i] = temp * 2 + i;
        
        /* WAR hazard: write after read */
        float ftemp = fdata[i];
        fdata[i] = ftemp * 1.5f;
        
        /* WAW hazard: write after write */
        barrier = data[i];
        data[i] = barrier + 1;
        
        sum += data[i];
        
        /* Scheduling barrier */
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static float cold_function(float *a, float *b, int n) {
    float result = 0.0f;
    
    /* SIMD-friendly loop with unrolling hint */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Mixed FP and integer operations */
        float x = a[i] * b[i];
        int ix = (int)x;
        float y = x + (float)ix;
        
        /* Complex expression with dependencies */
        result += y * sinf((float)i * 0.01f);
        
        /* Assembly with register clobber */
        asm volatile("" : "+r"(result) : : "r0", "r1", "cc");
    }
    return result;
}

__attribute__((optimize("O3")))
static int pointer_chasing(int *array, int n) {
    int *ptr = array;
    int sum = 0;
    
    /* Pointer chasing with varying strides */
    for (int i = 0; i < n; i++) {
        /* Load/store sequence */
        int val = *ptr;
        *ptr = val + i;
        
        /* Update pointer with complex addressing */
        ptr = array + ((i * 13) & (n - 1)); /* Power of two mask */
        
        sum += val;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((noinline))
static int complex_control_flow(int x) {
    int result = 0;
    
    /* Switch with sparse cases */
    switch (x % 7) {
        case 0:
            result = x * 2;
            /* Fall through */
        case 1:
            result += x / 3;
            break;
        case 2:
            result = x | 0xFF;
            break;
        case 3:
            /* Conditional move */
            result = (x > 100) ? x : -x;
            break;
        case 4:
            result = x ^ (x >> 3);
            break;
        case 5:
            result = x * x;
            break;
        default:
            result = ~x;
            break;
    }
    
    /* Nested if-else with early returns */
    if (result < 0) {
        if (result < -1000) return -1;
        result = -result;
    } else if (result > 1000) {
        return 1;
    }
    
    /* Loop with multiple exit points */
    for (int i = 0; i < 10; i++) {
        if (result & (1 << i)) {
            result += i;
            if (result > 500) break;
        } else {
            result -= i;
            if (result < -500) continue;
        }
        
        /* Mixed operation */
        result = (result * 3) / 2;
    }
    
    return result;
}

__attribute__((optimize("O3")))
static void vectorized_loop(float *a, float *b, float *c, int n) {
    /* Auto-vectorizable loop */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* SIMD operations */
        float x = a[i] * 2.5f;
        float y = b[i] * 1.5f;
        c[i] = x + y - sqrtf(fabsf(x - y));
        
        /* Integer operation in FP loop */
        int idx = i & 0xF;
        c[i] += (float)idx * 0.1f;
    }
}

__attribute__((optimize("O3")))
static int mixed_operations_test(int *idata, float *fdata, double *ddata, int n) {
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* Loop with heterogeneous operations */
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int ival = idata[i];
        ival = (ival * 3 + 7) & 0xFF;
        int_sum += ival;
        
        /* Floating point operations */
        float fval = fdata[i];
        fval = fval * 2.0f + sinf(fval);
        float_sum += fval;
        
        /* Double precision operations */
        double dval = ddata[i];
        dval = dval * 1.5 + cos(dval);
        double_sum += dval;
        
        /* Cross-type conversions */
        idata[i] = (int)(fval + dval);
        fdata[i] = (float)(ival + dval);
        ddata[i] = (double)(ival + fval);
        
        /* Scheduling barrier with clobber */
        asm volatile("" : "+r"(int_sum), "+r"(float_sum) : : "r2", "r3", "memory");
    }
    
    return int_sum + (int)float_sum + (int)double_sum;
}

int main(void) {
    /* Allocate aligned memory for better vectorization */
    int *int_data = (int*)aligned_alloc(64, SIZE * sizeof(int));
    float *float_data = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *float_data2 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *float_data3 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    double *double_data = (double*)aligned_alloc(64, SIZE * sizeof(double));
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (float)(rand() % 1000) / 10.0f;
        float_data2[i] = (float)(rand() % 1000) / 20.0f;
        float_data3[i] = 0.0f;
        double_data[i] = (double)(rand() % 1000) / 30.0;
    }
    
    int total_result = 0;
    
    /* Execute test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with scheduling challenges */
        total_result += hot_function(int_data, float_data, SIZE);
        
        /* Call cold function with different optimization */
        total_result += (int)cold_function(float_data, float_data2, SIZE);
        
        /* Pointer chasing test */
        total_result += pointer_chasing(int_data, SIZE);
        
        /* Complex control flow test */
        for (int i = 0; i < 50; i++) {
            total_result += complex_control_flow(int_data[i % SIZE]);
        }
        
        /* Vectorized loop */
        vectorized_loop(float_data, float_data2, float_data3, SIZE);
        total_result += (int)float_data3[SIZE / 2];
        
        /* Mixed operations test */
        total_result += mixed_operations_test(int_data, float_data, double_data, SIZE);
        
        /* Prevent loop invariant code motion */
        asm volatile("" ::: "memory");
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    printf("Sample values: int=%d, float=%.2f, double=%.2f\n", 
           int_data[0], float_data[0], double_data[0]);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(float_data2);
    free(float_data3);
    free(double_data);
    
    return total_result != 0 ? 0 : 1;
}
