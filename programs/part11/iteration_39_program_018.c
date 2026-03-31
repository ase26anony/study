/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that force the scheduler to allocate and use the full
 * scheduling context, including target-specific hooks, frontend state
 * saving, large instruction queues, and ready lists.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* Inline functions to increase instruction count */
static ALWAYS_INLINE int compute_int_chain(int a, int b, int c) {
    /* Create dependency chain for scheduling pressure */
    int t1 = a * b + c;
    int t2 = t1 ^ (t1 >> 3);
    int t3 = t2 * 0x5bd1e995;
    int t4 = t3 + (t3 << 15);
    return t4 ^ (t4 >> 13);
}

static ALWAYS_INLINE float compute_float_chain(float x, float y, float z) {
    /* Mixed FP operations using different execution units */
    float a = x * y + z;
    float b = sinf(a) * cosf(a);
    float c = b * expf(b);
    float d = c / (1.0f + fabsf(c));
    return d;
}

static ALWAYS_INLINE double compute_double_chain(double x, double y) {
    /* Double precision chain */
    double a = x * y - global_double;
    double b = a * a + 2.0 * a;
    double c = sqrt(fabs(b)) + 1.0;
    double d = log(c + 1.0);
    return d * d - 2.0 * d;
}

/* Function with memory aliasing to prevent reordering */
static ALWAYS_INLINE void memory_aliasing_ops(int* arr1, int* arr2, int size) {
    for (int i = 1; i < size - 1; i++) {
        /* Potential aliasing creates scheduling barriers */
        arr1[i] = arr1[i-1] + arr2[i];
        arr2[i+1] = arr1[i] * arr2[i-1];
        arr1[i] ^= arr2[i];
    }
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int speculative_computation(int x, int limit) {
    int result = 0;
    
    /* Inner loop with small iteration count for software pipelining */
    for (int i = 0; i < 6; i++) {
        /* Dependent operations creating scheduling pressure */
        int a = x * i + global_seed;
        int b = a ^ (a << 3);
        int c = b * 0x9e3779b9;
        
        /* Conditional that might be speculatively scheduled */
        if (c % 3 == 0) {
            result += c * 2;
        } else if (c % 5 == 0) {
            result += c / 2;
        } else {
            result += c;
        }
        
        /* More dependencies */
        x = (x * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final conditional branch */
    if (result > limit) {
        result = result % limit;
    }
    
    return result;
}

/* Wide basic block with many independent computation paths */
static ALWAYS_INLINE void wide_basic_block(int* results, float* fresults, 
                                          double* dresults, int n) {
    /* Multiple independent chains to fill ready list */
    for (int i = 0; i < n; i++) {
        /* Chain 1: Integer operations */
        int a = i * 3 + 1;
        int b = a << 2;
        int c = b ^ 0x55aa55aa;
        int d = c * 0xdeece66d;
        
        /* Chain 2: Floating point operations */
        float x = i * 0.1f;
        float y = sinf(x) * 2.0f;
        float z = y * y - 1.0f;
        float w = z / (z + 2.0f);
        
        /* Chain 3: Double precision */
        double p = i * 0.01;
        double q = p * p + global_double;
        double r = sqrt(q);
        double s = log(r + 1.0);
        
        /* Chain 4: More integer with bit operations */
        int e = d ^ i;
        int f = e * 0x5a827999;
        int g = f + (f << 8);
        int h = g ^ (g >> 4);
        
        /* Chain 5: Mixed operations */
        double t = s * w + i;
        float u = t * 0.5f;
        int v = (int)(u * 1000.0f);
        
        /* Store results (memory ops create scheduling barriers) */
        results[i] = d + h;
        fresults[i] = w * u;
        dresults[i] = s * t;
        
        /* Additional independent computation */
        int k = v * 7;
        float m = k * 0.003f;
        results[i] ^= (int)(m * 1000);
    }
}

/* Function using vector extensions for parallel operations */
static ALWAYS_INLINE v4si vector_computation(v4si a, v4si b, v4si c) {
    /* Vector operations expand to multiple parallel instructions */
    v4si t1 = a * b + c;
    v4si t2 = t1 << 2;
    v4si t3 = t2 ^ b;
    v4si t4 = t3 * a;
    v4si t5 = t4 + (t4 >> 1);
    return t5;
}

/* Function with switch statement for complex control flow */
static ALWAYS_INLINE int switch_based_computation(int x, int mode) {
    int result = x;
    
    /* Switch creates control flow requiring state tracking */
    switch (mode % 8) {
        case 0:
            result = compute_int_chain(x, x+1, x+2);
            break;
        case 1:
            result = x * 3 - 7;
            break;
        case 2:
            result = (x << 4) | (x >> 4);
            break;
        case 3:
            result = x ^ 0x12345678;
            break;
        case 4:
            result = x * x - x;
            break;
        case 5:
            result = (x * 0x9e3779b9) >> 16;
            break;
        case 6:
            result = x + (x << 8) + (x << 16);
            break;
        case 7:
            result = ~x;
            break;
    }
    
    /* Additional computation after switch */
    for (int i = 0; i < 4; i++) {
        result = (result * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return result;
}

/* Main test function 1: Complex arithmetic with dependencies */
void test_function_1(int iterations) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    /* Large basic block created by loop unrolling */
    #pragma GCC unroll 8
    for (int i = 0; i < iterations; i++) {
        /* Multiple dependent operations */
        int a = i * global_seed;
        int b = compute_int_chain(a, i, global_seed);
        int c = b ^ (b << 3);
        
        /* Floating point chain */
        float x = i * 0.01f + global_float;
        float y = compute_float_chain(x, x*2.0f, x*0.5f);
        float z = y * y - 2.0f * y + 1.0f;
        
        /* Double precision chain */
        double p = i * 0.001 + global_double;
        double q = compute_double_chain(p, p * 1.5);
        double r = q / (1.0 + fabs(q));
        
        /* Memory operations */
        sum_int += c;
        sum_float += z;
        sum_double += r;
        
        /* More operations to increase instruction count */
        int d = c * 0x5a827999;
        float s = sinf(z) * cosf(z);
        double t = r * exp(r);
        
        sum_int ^= d;
        sum_float *= (s + 1.0f);
        sum_double -= t * 0.5;
    }
    
    /* Conditional at end of block */
    if (sum_int > 1000000) {
        sum_int = sum_int % 1000000;
    }
    
    printf("Test 1: int=%d float=%.3f double=%.3f\n", 
           sum_int, sum_float, sum_double);
}

/* Main test function 2: Memory intensive with aliasing */
void test_function_2(int size) {
    int* arr1 = malloc(size * sizeof(int));
    int* arr2 = malloc(size * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7 + 1;
    }
    
    /* Perform memory operations with potential aliasing */
    for (int iter = 0; iter < 10; iter++) {
        memory_aliasing_ops(arr1, arr2, size);
        
        /* Additional computations between memory ops */
        for (int i = 0; i < size; i += 4) {
            /* Vector-like operations */
            int a = arr1[i] * arr2[i];
            int b = arr1[i+1] + arr2[i+1];
            int c = arr1[i+2] ^ arr2[i+2];
            int d = arr1[i+3] - arr2[i+3];
            
            arr1[i] = a ^ b;
            arr2[i+1] = c * d;
            arr1[i+2] = b + c;
            arr2[i+3] = d - a;
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum ^= arr1[i] + arr2[i];
    }
    
    printf("Test 2: checksum=%d\n", checksum);
    
    free(arr1);
    free(arr2);
}

/* Main test function 3: Speculative scheduling and switches */
void test_function_3(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Function with speculative opportunities */
        int a = speculative_computation(i, 1000);
        
        /* Switch-based computation */
        int b = switch_based_computation(a, i);
        
        /* More complex conditional logic */
        int c;
        if (a % 2 == 0) {
            c = compute_int_chain(a, b, i);
        } else if (a % 3 == 0) {
            c = a * b - i;
        } else if (a % 5 == 0) {
            c = (a << 3) | (b >> 2);
        } else {
            c = a ^ b ^ i;
        }
        
        /* Additional loop with small iteration count */
        for (int j = 0; j < 5; j++) {
            c = (c * 1103515245 + 12345) & 0x7fffffff;
            if (c % 7 == 0) {
                c += j * 1000;
            }
        }
        
        total += c;
    }
    
    printf("Test 3: total=%d\n", total);
}

/* Main test function 4: Wide basic blocks and vector operations */
void test_function_4(int n) {
    int* int_results = malloc(n * sizeof(int));
    float* float_results = malloc(n * sizeof(float));
    double* double_results = malloc(n * sizeof(double));
    
    /* Create wide basic block with many independent operations */
    wide_basic_block(int_results, float_results, double_results, n);
    
    /* Vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    
    for (int i = 0; i < 10; i++) {
        vec_a = vector_computation(vec_a, vec_b, vec_c);
        vec_b = vector_computation(vec_b, vec_c, vec_a);
        vec_c = vector_computation(vec_c, vec_a, vec_b);
    }
    
    /* Mix vector and scalar results */
    int vec_sum = 0;
    for (int i = 0; i < 4; i++) {
        vec_sum += vec_a[i] + vec_b[i] + vec_c[i];
    }
    
    /* Compute final result */
    int final_sum = vec_sum;
    for (int i = 0; i < n; i++) {
        final_sum += int_results[i] + (int)float_results[i] + (int)double_results[i];
    }
    
    printf("Test 4: final_sum=%d\n", final_sum);
    
    free(int_results);
    free(float_results);
    free(double_results);
}

/* Main test function 5: Mixed operations with function calls */
void test_function_5(int iterations) {
    double accumulator = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression with mixed types */
        int a = i * 137;
        float b = sinf(i * 0.01f) * 100.0f;
        double c = cos(i * 0.005) * 200.0;
        
        /* Multiple dependent operations */
        int d = compute_int_chain(a, (int)b, (int)c);
        float e = compute_float_chain(b, c, d);
        double f = compute_double_chain(c, d + e);
        
        /* Inline assembly to create scheduling barrier */
        asm volatile("" : "+r" (d), "+r" (a) : : "memory");
        
        /* More computations */
        int g = d * a ^ 0x55aa55aa;
        float h = e * e - 2.0f * e + 1.0f;
        double j = f * f + f * 2.0 + 1.0;
        
        /* Conditional updates */
        if (g % 11 == 0) {
            accumulator += g * 0.1 + h + j;
        } else if (g % 13 == 0) {
            accumulator -= h * 0.5 + j * 0.3;
        } else {
            accumulator *= 0.99;
            accumulator += (g % 100) * 0.01;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r" (accumulator) : "memory");
    }
    
    printf("Test 5: accumulator=%.3f\n", accumulator);
}

/* Main driver function */
int main(int argc, char** argv) {
    int iterations = 1000;
    int size = 256;
    
    printf("Starting scheduler coverage tests...\n");
    
    /* Run all test functions to trigger different scheduling scenarios */
    test_function_1(iterations);      /* Complex arithmetic dependencies */
    test_function_2(size);            /* Memory aliasing operations */
    test_function_3(iterations / 10); /* Speculative scheduling */
    test_function_4(size / 4);        /* Wide basic blocks and vectors */
    test_function_5(iterations / 2);  /* Mixed operations with barriers */
    
    /* Additional complex computation to ensure coverage */
    int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result = compute_int_chain(final_result, i, global_seed);
        final_result ^= switch_based_computation(final_result, i);
        final_result = speculative_computation(final_result, 10000);
    }
    
    printf("Final result: %d\n", final_result);
    printf("All tests completed.\n");
    
    return 0;
}
