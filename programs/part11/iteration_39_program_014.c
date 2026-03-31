/* test_sched_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that force the scheduler to allocate and use the full
 * scheduling context, including target-specific hooks and frontend state
 * saving.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic with dependencies to create scheduling pressure */
static ALWAYS_INLINE double complex_math_chain(double a, double b, double c, double d) {
    /* Long dependency chain */
    double t1 = a * b + c;
    double t2 = t1 / (d + 1.0);
    double t3 = sin(t2) * cos(t1);
    double t4 = t3 * t3 - t2 * t2;
    double t5 = sqrt(fabs(t4)) + exp(t3);
    double t6 = log(fabs(t5) + 1.0) * tan(t4);
    double t7 = t6 * t6 - t5 * t5;
    double t8 = asin(fmod(fabs(t7), 0.99)) * acos(fmod(fabs(t6), 0.99));
    return t8;
}

/* Mixed integer and floating point operations */
static ALWAYS_INLINE int mixed_operations(int* arr, float* farr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    
    /* Create independent chains that can be scheduled in parallel */
    for (int i = 0; i < n; i++) {
        /* Integer chain */
        int val1 = arr[i] * 3;
        int val2 = val1 + (i << 2);
        int val3 = val2 ^ (val1 >> 3);
        sum_int += val3;
        
        /* Floating point chain */
        float fval1 = farr[i] * 2.5f;
        float fval2 = fval1 + sinf(i * 0.1f);
        float fval3 = fval2 * cosf(fval1);
        sum_float += fval3;
        
        /* Cross dependency */
        arr[i] = (int)(fval3 * 100) + val3;
    }
    
    return sum_int + (int)sum_float;
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int speculative_computation(int x, int y, int* branch_taken) {
    int result = 0;
    
    /* Multiple conditional updates creating scheduling barriers */
    if (x > y) {
        result = x * y - (x << 3);
        *branch_taken = 1;
    } else if (x < y) {
        result = y / (x + 1) + (y >> 2);
        *branch_taken = 2;
    } else {
        result = x ^ y;
        *branch_taken = 3;
    }
    
    /* More conditionals to create complex control flow */
    if (result > 1000) {
        result = result % 1000;
    } else if (result < -1000) {
        result = -result % 1000;
    }
    
    return result;
}

/* Wide basic block with many independent operations */
static ALWAYS_INLINE void wide_basic_block(int* out, const int* in, int size) {
    /* Unrolled operations to create wide basic block */
    for (int i = 0; i < size; i += 8) {
        /* 8 independent chains */
        int t0 = in[i] * 3 + 1;
        int t1 = in[i+1] * 5 - 2;
        int t2 = in[i+2] << 1;
        int t3 = in[i+3] >> 2;
        int t4 = in[i+4] ^ 0x55;
        int t5 = in[i+5] | 0xAA;
        int t6 = in[i+6] & 0xFF;
        int t7 = in[i+7] * 7 + 3;
        
        /* Cross dependencies */
        out[i] = t0 + t1;
        out[i+1] = t2 * t3;
        out[i+2] = t4 - t5;
        out[i+3] = t6 ^ t7;
        out[i+4] = t0 * t7;
        out[i+5] = t1 + t6;
        out[i+6] = t2 ^ t4;
        out[i+7] = t3 | t5;
    }
}

/* Vector operations for SIMD scheduling */
static ALWAYS_INLINE v4sf vector_operations(v4sf a, v4sf b, v4sf c) {
    /* Multiple vector operations creating parallel execution */
    v4sf r1 = a + b * c;
    v4sf r2 = a - b / (c + 1.0f);
    v4sf r3 = r1 * r2;
    v4sf r4 = __builtin_shuffle(r1, r2, (v4si){0, 1, 2, 3});
    v4sf r5 = r3 + r4;
    
    /* Conditional vector operations */
    v4sf mask = a > b;
    v4sf r6 = __builtin_shuffle(r5, c, (v4si){3, 2, 1, 0});
    v4sf result = mask ? r5 : r6;
    
    return result;
}

/* Function with software pipelining opportunities */
static ALWAYS_INLINE double software_pipelined_loop(int iterations) {
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < iterations; i++) {
        /* Multiple accumulators with dependencies */
        double t = sin(i * 0.01);
        acc1 = acc1 * 0.99 + t;
        acc2 = acc2 * 0.98 + cos(t);
        acc3 = acc3 * 0.97 + sqrt(fabs(acc1 + acc2));
    }
    
    return acc1 + acc2 + acc3;
}

/* Complex function with all scheduling characteristics */
static int test_function_1(int seed) {
    int data[64];
    float fdata[64];
    int result = seed;
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data[i] = (i * seed) & 0xFF;
        fdata[i] = (i * seed * 0.1f);
    }
    
    /* Mixed operations */
    result += mixed_operations(data, fdata, 64);
    
    /* Wide basic block */
    int out[64];
    wide_basic_block(out, data, 64);
    
    /* Process results with dependencies */
    for (int i = 0; i < 64; i++) {
        result += out[i] * (i + 1);
    }
    
    /* Speculative computation */
    int branch_taken;
    result ^= speculative_computation(result, seed, &branch_taken);
    
    /* Complex math chain */
    double dresult = complex_math_chain(result, seed * 0.5, seed * 0.3, seed * 0.7);
    result += (int)(dresult * 1000);
    
    return result;
}

/* Function emphasizing floating point and vector operations */
static float test_function_2(int seed) {
    float result = seed * 0.5f;
    
    /* Vector operations */
    v4sf vec_a = {result, result * 0.5f, result * 0.3f, result * 0.7f};
    v4sf vec_b = {sinf(result), cosf(result), tanf(result), expf(result)};
    v4sf vec_c = {logf(result + 1.0f), sqrtf(fabsf(result)), 
                  result * result, 1.0f / (result + 0.001f)};
    
    for (int i = 0; i < 16; i++) {
        v4sf vec_result = vector_operations(vec_a, vec_b, vec_c);
        /* Extract and accumulate */
        float temp[4];
        memcpy(temp, &vec_result, sizeof(temp));
        result += temp[0] + temp[1] + temp[2] + temp[3];
        
        /* Update vectors */
        vec_a = vec_b;
        vec_b = vec_c;
        vec_c = vec_result;
    }
    
    /* Software pipelined computation */
    result += software_pipelined_loop(8);
    
    /* More complex math */
    for (int i = 0; i < 32; i++) {
        double dval = complex_math_chain(result + i, i * 0.1, i * 0.2, i * 0.3);
        result += (float)dval * 0.01f;
    }
    
    return result;
}

/* Function with many small basic blocks and control flow */
static int test_function_3(int seed) {
    int result = 0;
    int data[128];
    
    /* Initialize with pattern */
    for (int i = 0; i < 128; i++) {
        data[i] = (i * seed + (i >> 3)) & 0x3FF;
    }
    
    /* Process with many conditionals */
    for (int i = 0; i < 128; i++) {
        int val = data[i];
        
        /* Multiple conditional chains */
        if (val < 100) {
            result += val * 3;
            if (val < 50) {
                result -= val * 2;
            } else {
                result += val / 2;
            }
        } else if (val < 200) {
            result += val << 1;
            if (val < 150) {
                result ^= val;
            } else {
                result |= val;
            }
        } else if (val < 300) {
            result += val >> 2;
            if (val < 250) {
                result &= val;
            } else {
                result *= 2;
            }
        } else {
            result += val % 137;
            if (val < 350) {
                result = (result << 3) | (val & 0xF);
            } else {
                result = (result >> 2) ^ val;
            }
        }
        
        /* Additional computation */
        result += (val * val) % 7919;
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main() {
    int total_result = 0;
    float float_result = 0.0f;
    
    /* Seed for reproducible results */
    int seed = time(NULL) % 1000;
    printf("Starting with seed: %d\n", seed);
    
    /* Call test functions multiple times to ensure scheduling happens */
    for (int iteration = 0; iteration < 10; iteration++) {
        int current_seed = seed + iteration * 17;
        
        /* Function 1: Integer and mixed operations */
        int r1 = test_function_1(current_seed);
        total_result ^= r1;
        printf("Iteration %d, test1 result: %d\n", iteration, r1);
        
        /* Function 2: Floating point and vector operations */
        float r2 = test_function_2(current_seed);
        float_result += r2;
        printf("Iteration %d, test2 result: %f\n", iteration, r2);
        
        /* Function 3: Control flow intensive */
        int r3 = test_function_3(current_seed);
        total_result += r3;
        printf("Iteration %d, test3 result: %d\n", iteration, r3);
        
        /* Additional complex computation to prevent optimization */
        double complex_val = complex_math_chain(r1, r2, r3, current_seed);
        total_result += (int)(complex_val * 100);
    }
    
    /* Final computation to use results */
    float_result /= 10.0f;
    total_result += (int)(float_result * 1000);
    
    printf("Final integer result: %d\n", total_result);
    printf("Final float result: %f\n", float_result);
    
    /* Return non-zero to indicate success */
    return total_result != 0 ? 0 : 1;
}
