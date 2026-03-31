/* sel-sched-test.c - Comprehensive test for GCC selective scheduler dump logic */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function with scheduling pressure attribute */
__attribute__((hot, optimize("O3", "sched-pressure")))
static float hot_loop_with_hazards(float* restrict a, float* restrict b, 
                                   float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed RAW, WAR, WAW hazards */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on previous a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: a reused */
        a[i] = temp + b[i];
        
        /* WAW hazard: multiple writes to sum */
        sum += a[i];
        
        /* Complex dependency chain */
        c[i] = sqrtf(fabsf(a[i] * b[i] + temp));
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
        
        /* Another dependent operation */
        b[i] = c[i] * 0.5f + sum;
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline))
static double cold_pointer_chasing(double* restrict base, int* restrict indices, int n) {
    double result = 0.0;
    double* current = base;
    
    /* Pointer chasing with varying strides */
    for (int i = 0; i < n; i++) {
        int idx = indices[i] % n;
        current = base + idx;
        
        /* Load with potential cache miss */
        double val = *current;
        
        /* Mixed integer/floating point ops */
        int ival = (int)(val * 100.0);
        result += val * ival;
        
        /* Store with dependency */
        *current = result * 0.01;
        
        /* Assembly with register clobber */
        asm volatile("" : "+r"(current) : : "r8", "r9", "memory");
    }
    
    return result;
}

/* Function with complex control flow */
__attribute__((optimize("O3")))
static int mixed_control_flow(int* data, int n) {
    int total = 0;
    
    #pragma GCC unroll 2
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Switch with sparse cases */
        switch (val & 0xF) {
            case 0:  val *= 2; break;
            case 1:  val += 100; break;
            case 5:  val -= 50; break;
            case 10: val /= 3; break;
            default: val ^= 0xFF; break;
        }
        
        /* Conditional move vs branch */
        int pred = (val > 1000) ? val : -val;
        
        /* Multiple exit points */
        if (pred < 0) {
            if (pred < -10000) continue;
            total += pred;
        } else {
            total -= pred;
        }
        
        /* Early exit condition */
        if (total > 1000000) break;
        
        /* Nested if-else */
        if (i % 3 == 0) {
            val = (val << 2) | 0x1;
        } else if (i % 3 == 1) {
            val = (val >> 1) & 0x7F;
        } else {
            val = val ^ (val << 4);
        }
        
        data[i] = val;
    }
    
    return total;
}

/* SIMD-friendly loop with vectorization hints */
__attribute__((optimize("O3", "tree-vectorize")))
static void vectorizable_loop(float* restrict src, float* restrict dst, 
                              float* restrict mask, int n) {
    /* Compile-time known size helps vectorization */
    float local_src[ARRAY_SIZE];
    float local_dst[ARRAY_SIZE];
    
    /* Copy to local arrays */
    for (int i = 0; i < n; i++) {
        local_src[i] = src[i];
    }
    
    /* Vectorizable computation */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* FMA-like operations */
        float x = local_src[i];
        float m = mask[i % 256];
        
        /* Polynomial evaluation (vectorizable) */
        float y = x * x * 0.5f + x * 1.5f + 2.0f;
        
        /* Conditional select */
        y = (x > 0.0f) ? y : -y;
        
        /* Store with possible aliasing break */
        asm volatile("" ::: "memory");
        
        local_dst[i] = y * m;
    }
    
    /* Copy back */
    for (int i = 0; i < n; i++) {
        dst[i] = local_dst[i];
    }
}

/* Function with computed goto (challenges scheduler) */
__attribute__((noinline, optimize("O2")))
static int computed_goto_pattern(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_default, &&case_5, &&case_6, &&case_7
    };
    
    int result = x;
    
    if (x >= 0 && x < 8) {
        goto *jump_table[x];
    } else {
        goto case_default;
    }
    
case_0:
    result *= 3;
    /* Fall through */
case_1:
    result += 11;
    goto end;
    
case_2:
    result -= 7;
    /* No break - intentional fallthrough */
case_3:
    result ^= 0xAA;
    goto end;
    
case_5:
    result /= 2;
    /* Fall through */
case_6:
    result <<= 1;
    goto end;
    
case_7:
    result |= 0xF0;
    goto end;
    
case_default:
    result = ~result;
    /* Memory barrier in middle of control flow */
    asm volatile("" ::: "memory");
    result += 100;
    
end:
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize data */
    float fa[ARRAY_SIZE], fb[ARRAY_SIZE], fc[ARRAY_SIZE];
    double da[ARRAY_SIZE];
    int indices[ARRAY_SIZE], idata[ARRAY_SIZE];
    float mask[256];
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 50.0f;
        fc[i] = 0.0f;
        da[i] = (double)rand() / RAND_MAX * 200.0;
        indices[i] = rand() % ARRAY_SIZE;
        idata[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 256; i++) {
        mask[i] = (float)(i % 10) * 0.1f;
    }
    
    float total_float = 0.0f;
    double total_double = 0.0;
    int total_int = 0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call all test functions with different scheduling characteristics */
        total_float += hot_loop_with_hazards(fa, fb, fc, ARRAY_SIZE);
        
        total_double += cold_pointer_chasing(da, indices, ARRAY_SIZE / 4);
        
        total_int += mixed_control_flow(idata, ARRAY_SIZE);
        
        vectorizable_loop(fa, fb, mask, ARRAY_SIZE);
        
        /* Update indices for pointer chasing */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            indices[i] = (indices[i] * 13 + 7) % ARRAY_SIZE;
        }
        
        /* Call computed goto function */
        total_int += computed_goto_pattern(iter % 10);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: float=%f double=%f int=%d\n", 
           total_float, total_double, total_int);
    
    /* Additional computation using results */
    float final_check = total_float + (float)total_double + (float)total_int;
    printf("Final check value: %f\n", final_check);
    
    return (final_check > 0.0f) ? 0 : 1;
}
