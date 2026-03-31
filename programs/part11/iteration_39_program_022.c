/* test_sched_coverage.c
 * 
 * This program is designed to trigger the cleanup logic in GCC's Haifa scheduler
 * (free_sched_block function in haifa-sched.cc lines 4681-4691).
 * It creates complex basic blocks that force the scheduler to allocate and use
 * the full scheduling context, including target-specific hooks, frontend state
 * saving, large instruction queues, and ready lists.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float_result = 0.0f;
volatile double global_double_result = 0.0;

/* ======================================================================
 * Test Function 1: Mixed Integer and FP Operations with Dependency Chains
 * This creates long dependency chains that require careful scheduling
 * ====================================================================== */
ALWAYS_INLINE static float complex_float_chain(float a, float b, float c, float d, float e) {
    /* Multiple dependent FP operations */
    float t1 = a * b + c;
    float t2 = t1 / d - e;
    float t3 = sinf(t2) * cosf(t1);
    float t4 = t3 + expf(t2 * 0.5f);
    float t5 = t4 * t4 - t3 * t3;
    float t6 = sqrtf(fabsf(t5)) + logf(fabsf(t4) + 1.0f);
    return t6;
}

ALWAYS_INLINE static int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Multiple dependent integer operations */
    int t1 = (a * b) >> 3;
    int t2 = (t1 + c) * d;
    int t3 = t2 ^ (t1 << 2);
    int t4 = (t3 % (e + 1)) + (t2 / (b + 1));
    int t5 = t4 * t4 - t3 * t3;
    int t6 = (t5 >> 4) | (t4 & 0xFF);
    return t6;
}

/* Function with speculative scheduling opportunities */
static float test_speculative_scheduling(int iterations) {
    float acc = 0.0f;
    int i;
    
    /* Loop with conditional updates - creates speculative scheduling opportunities */
    for (i = 0; i < iterations; i++) {
        float base = (float)i;
        float val;
        
        /* Complex conditional chain */
        if (i & 1) {
            val = complex_float_chain(base, base * 0.5f, base * 0.25f, 
                                     base * 0.125f, base * 0.0625f);
        } else if (i & 2) {
            val = complex_float_chain(base * 2.0f, base * 1.5f, base * 0.75f,
                                     base * 0.375f, base * 0.1875f);
        } else {
            val = complex_float_chain(base * 3.0f, base * 2.5f, base * 1.25f,
                                     base * 0.625f, base * 0.3125f);
        }
        
        /* More operations that depend on val */
        if (val > 0.0f) {
            acc += val * val;
        } else {
            acc -= val * val;
        }
        
        /* Integer operations mixed in */
        int int_val = complex_int_chain(i, i+1, i+2, i+3, i+4);
        acc += (float)(int_val & 0xFF) * 0.01f;
    }
    
    return acc;
}

/* ======================================================================
 * Test Function 2: Wide Basic Block with Unrolled Loop
 * Creates a large basic block to fill instruction queues
 * ====================================================================== */
static double wide_basic_block_unrolled(const double* input, int size) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
    double prod1 = 1.0, prod2 = 1.0;
    double max_val = -1e100, min_val = 1e100;
    
    /* Manual unrolling to create wide basic block */
    int i;
    for (i = 0; i < size; i += 8) {
        /* Load 8 values - creates multiple memory operations */
        double v0 = input[i];
        double v1 = input[i + 1];
        double v2 = input[i + 2];
        double v3 = input[i + 3];
        double v4 = input[i + 4];
        double v5 = input[i + 5];
        double v6 = input[i + 6];
        double v7 = input[i + 7];
        
        /* Independent computation chains - fills ready list */
        sum1 += v0 + v1 + v2 + v3;
        sum2 += v4 + v5 + v6 + v7;
        sum3 += v0 * v0 + v1 * v1 + v2 * v2 + v3 * v3;
        sum4 += v4 * v4 + v5 * v5 + v6 * v6 + v7 * v7;
        
        prod1 *= (v0 + 1.0) * (v1 + 1.0) * (v2 + 1.0) * (v3 + 1.0);
        prod2 *= (v4 + 1.0) * (v5 + 1.0) * (v6 + 1.0) * (v7 + 1.0);
        
        /* Conditional updates - requires state tracking */
        max_val = (v0 > max_val) ? v0 : max_val;
        max_val = (v1 > max_val) ? v1 : max_val;
        max_val = (v2 > max_val) ? v2 : max_val;
        max_val = (v3 > max_val) ? v3 : max_val;
        max_val = (v4 > max_val) ? v4 : max_val;
        max_val = (v5 > max_val) ? v5 : max_val;
        max_val = (v6 > max_val) ? v6 : max_val;
        max_val = (v7 > max_val) ? v7 : max_val;
        
        min_val = (v0 < min_val) ? v0 : min_val;
        min_val = (v1 < min_val) ? v1 : min_val;
        min_val = (v2 < min_val) ? v2 : min_val;
        min_val = (v3 < min_val) ? v3 : min_val;
        min_val = (v4 < min_val) ? v4 : min_val;
        min_val = (v5 < min_val) ? v5 : min_val;
        min_val = (v6 < min_val) ? v6 : min_val;
        min_val = (v7 < min_val) ? v7 : min_val;
        
        /* Mixed FP and integer operations */
        int idx0 = (int)v0;
        int idx1 = (int)v1;
        sum1 += (idx0 & 0xF) * 0.01;
        sum2 += (idx1 & 0xF) * 0.01;
    }
    
    /* Final reduction with more operations */
    double final_sum = sum1 + sum2 + sum3 + sum4;
    double final_prod = prod1 * prod2;
    double range = max_val - min_val;
    
    return final_sum * final_prod / (range + 1.0);
}

/* ======================================================================
 * Test Function 3: Vector Operations with SIMD-like Instructions
 * Triggers target-specific scheduling hooks for vector operations
 * ====================================================================== */
static v4sf vector_operations(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Multiple vector operations - uses SIMD execution units */
    v4sf t1 = a + b;
    v4sf t2 = c * d;
    v4sf t3 = t1 - t2;
    v4sf t4 = t3 * t3;
    v4sf t5 = __builtin_ia32_sqrtps(t4);  /* SSE intrinsic */
    v4sf t6 = t5 + a * b * c * d;
    
    /* Conditional vector operations */
    v4sf mask = t6 > t1;
    v4sf t7 = __builtin_ia32_blendvps(t6, t1, mask);  /* SSE4.1 intrinsic */
    
    /* More arithmetic */
    v4sf t8 = t7 + __builtin_ia32_rcpps(t4);  /* Reciprocal approximation */
    
    return t8;
}

static void test_vector_scheduling(void) {
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf c = {2.0f, 3.0f, 4.0f, 5.0f};
    v4sf d = {1.0f, 1.0f, 1.0f, 1.0f};
    
    v4sf result = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop with vector operations - creates scheduling pressure */
    for (int i = 0; i < 100; i++) {
        result = vector_operations(a, b, c, d);
        
        /* Update vectors for next iteration */
        a = result + b;
        b = b * 1.1f;
        c = c - 0.1f;
        d = d + 0.05f;
        
        /* Mix with scalar operations */
        float scalar = ((float*)&result)[i % 4];
        global_float_result += scalar;
    }
}

/* ======================================================================
 * Test Function 4: Memory Operations with Potential Aliasing
 * Creates scheduling barriers due to memory dependencies
 * ====================================================================== */
static double memory_aliasing_test(double* arr1, double* arr2, int size) {
    double sum = 0.0;
    
    /* Complex memory access pattern with potential aliasing */
    for (int i = 1; i < size - 1; i++) {
        /* These could alias - creates memory dependencies */
        arr1[i] = arr1[i-1] * 0.9 + arr1[i+1] * 0.1;
        arr2[i] = arr2[i-1] * 0.8 + arr2[i+1] * 0.2;
        
        /* Cross-array operations */
        double temp = arr1[i] * arr2[i];
        arr1[i] = temp + sin(arr1[i]);
        arr2[i] = temp * cos(arr2[i]);
        
        /* Reduction with mixed operations */
        sum += arr1[i] + arr2[i];
        
        /* Pointer arithmetic that might alias */
        double* ptr1 = arr1 + (i % 16);
        double* ptr2 = arr2 + ((i * 7) % 16);
        *ptr1 += 0.01;
        *ptr2 -= 0.01;
    }
    
    return sum;
}

/* ======================================================================
 * Test Function 5: Function Calls with Side Effects
 * Creates scheduling barriers requiring state saving
 * ====================================================================== */
/* Volatile function to prevent inlining and create call barriers */
static volatile int (*volatile_func_ptr)(int) = NULL;

static int func_with_side_effects(int x) {
    /* Access volatile global */
    int seed = global_seed;
    
    /* System call or I/O simulation */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    int time_part = ts.tv_nsec & 0xFF;
    
    /* Complex computation */
    int result = x ^ seed ^ time_part;
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    /* Update global */
    global_seed = result & 0xFF;
    
    return result;
}

static void test_function_call_barriers(int iterations) {
    volatile_func_ptr = func_with_side_effects;
    
    int acc = 0;
    for (int i = 0; i < iterations; i++) {
        /* Function call creates scheduling barrier */
        int val = volatile_func_ptr(i);
        
        /* Computation around the call */
        int pre = i * i - i;
        int post = val * val + val;
        
        /* Dependent operations */
        acc += pre * post;
        acc ^= (val >> 4);
        
        /* More operations */
        if (acc & 1) {
            acc += func_with_side_effects(acc);
        } else {
            acc -= func_with_side_effects(acc >> 1);
        }
    }
    
    global_seed = acc & 0xFF;
}

/* ======================================================================
 * Test Function 6: Switch Statement with Multiple Cases
 * Creates complex control flow for state tracking
 * ====================================================================== */
static double switch_based_computation(int mode, double x, double y) {
    double result = 0.0;
    
    switch (mode % 8) {
        case 0:
            result = x + y;
            result = sin(result) * cos(x - y);
            break;
        case 1:
            result = x * y;
            result = exp(result) - log(fabs(x) + fabs(y) + 1.0);
            break;
        case 2:
            result = x / (y + 1.0);
            result = tan(result) * atan(x * y);
            break;
        case 3:
            result = sqrt(x*x + y*y);
            result = result * result - x * y;
            break;
        case 4:
            result = pow(x, y * 0.5);
            result = fmod(result, 100.0) + remainder(x, y);
            break;
        case 5:
            result = hypot(x, y);
            result = result + copysign(x, y) - copysign(y, x);
            break;
        case 6:
            result = fma(x, y, x + y);
            result = result * result - x * x - y * y;
            break;
        case 7:
            result = (x > y) ? x - y : y - x;
            result = log1p(fabs(result)) * expm1(fabs(x) + fabs(y));
            break;
    }
    
    /* Additional computation after switch */
    for (int i = 0; i < 4; i++) {
        result += complex_float_chain(result, x, y, (double)i, (double)(mode + i));
    }
    
    return result;
}

/* ======================================================================
 * Main Driver Function
 * Calls all test functions to exercise scheduler
 * ====================================================================== */
int main(void) {
    printf("Starting scheduler coverage test...\n");
    
    /* Initialize data */
    const int DATA_SIZE = 1024;
    double* data1 = (double*)malloc(DATA_SIZE * sizeof(double));
    double* data2 = (double*)malloc(DATA_SIZE * sizeof(double));
    
    srand(42);
    for (int i = 0; i < DATA_SIZE; i++) {
        data1[i] = (double)rand() / RAND_MAX * 100.0;
        data2[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    double total_result = 0.0;
    
    /* Test 1: Speculative scheduling with mixed operations */
    printf("Running Test 1: Speculative scheduling...\n");
    float test1_result = test_speculative_scheduling(100);
    total_result += test1_result;
    printf("  Test 1 result: %f\n", test1_result);
    
    /* Test 2: Wide basic block with unrolled operations */
    printf("Running Test 2: Wide basic block...\n");
    double test2_result = wide_basic_block_unrolled(data1, DATA_SIZE);
    total_result += test2_result;
    printf("  Test 2 result: %f\n", test2_result);
    
    /* Test 3: Vector operations with SIMD */
    printf("Running Test 3: Vector operations...\n");
    test_vector_scheduling();
    total_result += global_float_result;
    printf("  Test 3 result: %f\n", global_float_result);
    
    /* Test 4: Memory aliasing */
    printf("Running Test 4: Memory aliasing...\n");
    double test4_result = memory_aliasing_test(data1, data2, DATA_SIZE);
    total_result += test4_result;
    printf("  Test 4 result: %f\n", test4_result);
    
    /* Test 5: Function call barriers */
    printf("Running Test 5: Function call barriers...\n");
    test_function_call_barriers(50);
    total_result += global_seed;
    printf("  Test 5 result: %d\n", global_seed);
    
    /* Test 6: Switch-based computation */
    printf("Running Test 6: Switch-based computation...\n");
    double test6_result = 0.0;
    for (int i = 0; i < 100; i++) {
        test6_result += switch_based_computation(i, data1[i % DATA_SIZE], 
                                                data2[i % DATA_SIZE]);
    }
    total_result += test6_result;
    printf("  Test 6 result: %f\n", test6_result);
    
    /* Final computation to use all results */
    printf("\nTotal combined result: %f\n", total_result);
    
    /* Verify result to prevent dead code elimination */
    if (total_result > 1e6) {
        printf("Result verification: PASS (large result computed)\n");
    } else {
        printf("Result verification: WARNING (result may have been optimized away)\n");
    }
    
    free(data1);
    free(data2);
    
    return 0;
}
