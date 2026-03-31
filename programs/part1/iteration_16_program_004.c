#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling pressure */
__attribute__((hot, optimize("O3", "sched-pressure"))) 
static float hot_function(float* restrict a, float* restrict b, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read a[i], write to temp, read temp */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: write to a[i] after reading it */
        a[i] = temp + b[i];
        
        /* WAW hazard: multiple writes to sum */
        sum += a[i];
        sum = sum * 0.99f;  /* Another write to sum */
        
        /* Pointer chasing pattern */
        float* ptr = &a[i];
        *ptr = *ptr + 1.0f;
    }
    
    /* Memory barrier forcing scheduler decisions */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_function(double* arr, int size) {
    double result = 0.0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < size; i++) {
        switch (i % 7) {
            case 0: result += arr[i] * 2.0; break;
            case 1: result -= arr[i] / 3.0; break;
            case 2: result *= 1.1; break;
            case 3: result = (result > 0) ? result : -result; /* conditional move */
            case 4: /* fallthrough */
            case 5: result += sin(arr[i]); break;
            default: result = sqrt(fabs(result)); break;
        }
        
        /* Early exit condition */
        if (result > 1000000.0) {
            break;
        }
        
        /* Continue with another condition */
        if (i % 13 == 0) {
            continue;
        }
        
        /* Mixed operations */
        result = result + (double)i * 0.01;
    }
    
    return result;
}

/* SIMD-friendly function with unrolling pragma */
__attribute__((optimize("O3")))
static void vectorized_loop(float* restrict in, float* restrict out, int n) {
    int i;
    
    #pragma GCC unroll 4
    for (i = 0; i < (n & ~3); i += 4) {
        /* SIMD-friendly operations */
        out[i] = in[i] * 2.0f + 1.0f;
        out[i+1] = in[i+1] * 3.0f - 2.0f;
        out[i+2] = in[i+2] * 1.5f + 0.5f;
        out[i+3] = in[i+3] * 0.5f - 1.0f;
        
        /* Cross-lane dependencies */
        out[i] += out[i+1] * 0.1f;
        out[i+2] += out[i] * 0.2f;
    }
    
    /* Remainder loop */
    for (; i < n; i++) {
        out[i] = in[i] * 2.0f;
    }
    
    /* Assembly with register clobbers */
    asm volatile(
        "mov $0, %%eax\n\t"
        "cpuid\n\t"
        : 
        : 
        : "%eax", "%ebx", "%ecx", "%edx", "memory"
    );
}

/* Function with mixed data types and complex dependencies */
__attribute__((optimize("O3")))
static int mixed_operations(int* int_arr, float* float_arr, double* double_arr, int n) {
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* Nested loops with varying dependencies */
    for (int i = 0; i < n; i++) {
        /* Integer operations with pointer chasing */
        int* ptr = &int_arr[i];
        int val = *ptr;
        
        /* Multiple dependent operations */
        val = val * 3 + 7;
        val = val >> 2;
        val = val | (val << 16);
        
        /* Store with potential WAW hazard */
        int_arr[i] = val;
        int_sum += val;
        
        /* Floating point operations interleaved */
        float fval = float_arr[i];
        fval = fval * 3.14f + 2.71f;
        
        /* Memory barrier between dependent operations */
        asm volatile("" ::: "memory");
        
        fval = sinf(fval) * cosf(fval);
        float_arr[i] = fval;
        float_sum += fval;
        
        /* Double precision operations */
        double dval = double_arr[i];
        dval = dval * 1.41421356 + 3.14159265;
        
        /* Conditional operation creating control dependency */
        dval = (dval > 0.0) ? sqrt(dval) : sqrt(-dval);
        
        double_arr[i] = dval;
        double_sum += dval;
        
        /* Complex condition with early continue */
        if ((i % 17) == 0) {
            continue;
        }
        
        /* More operations after continue */
        int_sum ^= i;
        float_sum = float_sum * 0.999f;
    }
    
    /* Final reduction with memory barrier */
    asm volatile("" ::: "memory");
    
    return int_sum + (int)float_sum + (int)double_sum;
}

/* Computed goto pattern for complex control flow */
__attribute__((noinline))
static int computed_goto_test(int x) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    int result = x;
    
    if (x < 0 || x > 7) goto default_label;
    
    goto *jump_table[x];
    
label0:
    result = result * 2;
    /* fallthrough */
label1:
    result = result + 3;
    goto end;
label2:
    result = result - 5;
    goto end;
label3:
    result = result * result;
    goto end;
label4:
    result = result >> 1;
    goto end;
label5:
    result = result | 0xFF;
    goto end;
label6:
    result = result & 0x0F;
    goto end;
label7:
    result = ~result;
    goto end;
default_label:
    result = 0;
end:
    return result;
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* farr1 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* farr2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* farr3 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    int* iarr = (int*)aligned_alloc(32, SIZE * sizeof(int));
    double* darr = (double*)aligned_alloc(32, SIZE * sizeof(double));
    
    /* Initialize arrays */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
        farr3[i] = (float)rand() / RAND_MAX;
        iarr[i] = rand();
        darr[i] = (double)rand() / RAND_MAX;
    }
    
    float total = 0.0f;
    double dtotal = 0.0;
    int itotal = 0;
    
    /* Run multiple iterations to give scheduler work */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with scheduling pressure */
        total += hot_function(farr1, farr2, SIZE);
        
        /* Call cold function */
        dtotal += cold_function(darr, SIZE);
        
        /* Vectorized loop */
        vectorized_loop(farr1, farr3, SIZE);
        total += farr3[SIZE-1];
        
        /* Mixed operations */
        itotal += mixed_operations(iarr, farr2, darr, SIZE);
        
        /* Computed goto test */
        itotal += computed_goto_test(iter % 9);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Prevent dead code elimination */
    printf("Results: %f %f %d\n", total, dtotal, itotal);
    
    /* Cleanup */
    free(farr1);
    free(farr2);
    free(farr3);
    free(iarr);
    free(darr);
    
    return 0;
}
