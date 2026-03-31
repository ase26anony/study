/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that require extensive instruction scheduling with:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex instruction mixes
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

/* Complex arithmetic chains to create dependency chains */
static inline ALWAYS_INLINE double complex_chain_1(double a, double b, double c, double d) {
    /* Long dependency chain */
    double t1 = a * b + c;
    double t2 = t1 / (d + 1.0);
    double t3 = t2 * t2 - a;
    double t4 = sqrt(fabs(t3)) + b;
    double t5 = sin(t4) * cos(t4);
    double t6 = t5 * t5 - 2.0 * t5 + 1.0;
    return t6;
}

static inline ALWAYS_INLINE int integer_chain(int a, int b, int c) {
    /* Integer dependency chain */
    int t1 = a * b + c;
    int t2 = (t1 << 3) | (t1 >> 5);
    int t3 = t2 ^ (t2 * 7);
    int t4 = t3 % (abs(b) + 1);
    int t5 = t4 * t4 - t4;
    return t5;
}

/* Function with side effects to create scheduling barriers */
static inline ALWAYS_INLINE int barrier_function(int x) {
    /* Use inline assembly as a scheduling barrier */
    int result;
    asm volatile ("# Scheduling Barrier\n"
                  "movl %1, %0\n"
                  "# End Barrier"
                  : "=r" (result)
                  : "r" (x));
    return result;
}

/* Memory-intensive function with potential aliasing */
static inline ALWAYS_INLINE void memory_ops(int* restrict arr1, int* restrict arr2, 
                                           int* restrict arr3, int size) {
    /* Mixed memory operations */
    for (int i = 0; i < size; i++) {
        arr3[i] = arr1[i] * arr2[i];
        arr1[i] = arr3[i] + i;
        arr2[i] = arr1[i] ^ arr3[i];
    }
}

/* SIMD operations using GCC vector extensions */
static inline ALWAYS_INLINE v4sf simd_float_ops(v4sf a, v4sf b, v4sf c) {
    v4sf t1 = a * b + c;
    v4sf t2 = t1 / (a + 1.0f);
    v4sf t3 = __builtin_ia32_sqrtps(t2);  /* SSE intrinsic */
    v4sf t4 = t3 * b - c;
    return t4;
}

/* Function with speculative scheduling opportunities */
static int ALWAYS_INLINE speculative_ops(int x, int y, int* data, int n) {
    int result = 0;
    
    /* Complex conditional chain */
    if (x > 0) {
        result += integer_chain(x, y, data[0]);
        if (y < 100) {
            result += integer_chain(y, x, data[1]);
            if (x + y > 50) {
                result += barrier_function(data[2]);
            }
        }
    } else {
        result -= integer_chain(-x, y, data[3]);
    }
    
    /* Loop with small iteration count for software pipelining */
    for (int i = 0; i < 6; i++) {
        result += data[i % n] * i;
        result ^= (result << i);
    }
    
    return result;
}

/* Wide basic block with unrolled operations */
static double wide_basic_block(double a, double b, double c, double d, 
                              double e, double f, double g, double h) {
    /* Create many independent and dependent operations */
    double r1 = complex_chain_1(a, b, c, d);
    double r2 = complex_chain_1(b, c, d, e);
    double r3 = complex_chain_1(c, d, e, f);
    double r4 = complex_chain_1(d, e, f, g);
    double r5 = complex_chain_1(e, f, g, h);
    double r6 = complex_chain_1(f, g, h, a);
    double r7 = complex_chain_1(g, h, a, b);
    double r8 = complex_chain_1(h, a, b, c);
    
    /* Mix with integer operations */
    int i1 = integer_chain((int)r1, (int)r2, (int)r3);
    int i2 = integer_chain((int)r3, (int)r4, (int)r5);
    int i3 = integer_chain((int)r5, (int)r6, (int)r7);
    
    /* Memory operations */
    int arr1[8] = {i1, i2, i3, (int)r1, (int)r2, (int)r3, (int)r4, (int)r5};
    int arr2[8], arr3[8];
    memory_ops(arr1, arr2, arr3, 8);
    
    /* SIMD operations */
    v4sf va = { (float)r1, (float)r2, (float)r3, (float)r4 };
    v4sf vb = { (float)r5, (float)r6, (float)r7, (float)r8 };
    v4sf vc = { 1.0f, 2.0f, 3.0f, 4.0f };
    v4sf vresult = simd_float_ops(va, vb, vc);
    
    /* Final computation mixing all results */
    double final_result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    final_result += (double)i1 + i2 + i3;
    final_result += (double)arr3[0] + arr3[4];
    final_result += (double)vresult[0] + vresult[1] + vresult[2] + vresult[3];
    
    return final_result;
}

/* Function with switch statement for complex control flow */
static double control_flow_intensive(int mode, double x, double y) {
    double result = 0.0;
    
    switch (mode % 5) {
        case 0:
            result = complex_chain_1(x, y, x+y, x-y);
            /* Fall through */
        case 1:
            result += sin(x) * cos(y);
            result += barrier_function((int)x);
            break;
        case 2:
            result = exp(fabs(x - y));
            for (int i = 0; i < 4; i++) {
                result += complex_chain_1(result, x, y, i);
            }
            break;
        case 3:
            result = sqrt(x*x + y*y);
            /* Multiple independent chains */
            result += complex_chain_1(x, y, result, 1.0);
            result += complex_chain_1(y, x, result, 2.0);
            result += complex_chain_1(result, x, y, 3.0);
            break;
        case 4:
            /* Nested conditionals */
            if (x > y) {
                result = complex_chain_1(x, y, x/y, 0.0);
                if (result > 100.0) {
                    result = log(result);
                }
            } else {
                result = complex_chain_1(y, x, y/x, 0.0);
                result = pow(result, 0.5);
            }
            break;
    }
    
    return result;
}

/* Main test function with extremely wide basic block */
static double test_scheduler_wide_block(void) {
    /* Initialize with non-constant values to prevent optimization */
    double base = (double)clock() / CLOCKS_PER_SEC;
    
    /* Create a very wide basic block with unrolled operations */
    double sum = 0.0;
    
    /* Unrolled loop creating 64+ instructions */
    sum += wide_basic_block(base, base+1, base+2, base+3, 
                           base+4, base+5, base+6, base+7);
    sum += wide_basic_block(base+1, base+2, base+3, base+4,
                           base+5, base+6, base+7, base+8);
    sum += wide_basic_block(base+2, base+3, base+4, base+5,
                           base+6, base+7, base+8, base+9);
    sum += wide_basic_block(base+3, base+4, base+5, base+6,
                           base+7, base+8, base+9, base+10);
    sum += wide_basic_block(base+4, base+5, base+6, base+7,
                           base+8, base+9, base+10, base+11);
    sum += wide_basic_block(base+5, base+6, base+7, base+8,
                           base+9, base+10, base+11, base+12);
    sum += wide_basic_block(base+6, base+7, base+8, base+9,
                           base+10, base+11, base+12, base+13);
    sum += wide_basic_block(base+7, base+8, base+9, base+10,
                           base+11, base+12, base+13, base+14);
    
    /* Add speculative operations */
    int data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int spec_result = speculative_ops((int)sum, (int)base, data, 8);
    sum += (double)spec_result;
    
    /* Control flow intensive operations */
    sum += control_flow_intensive((int)sum % 5, sum, base);
    sum += control_flow_intensive((int)base % 5, base, sum);
    
    return sum;
}

/* Matrix multiplication to create many memory operations */
static void matrix_multiply(int n, double A[n][n], double B[n][n], double C[n][n]) {
    /* Triple nested loops create many scheduling opportunities */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            /* Unroll inner loop for wider basic block */
            for (int k = 0; k < n; k += 4) {
                sum += A[i][k] * B[k][j];
                if (k+1 < n) sum += A[i][k+1] * B[k+1][j];
                if (k+2 < n) sum += A[i][k+2] * B[k+2][j];
                if (k+3 < n) sum += A[i][k+3] * B[k+3][j];
            }
            C[i][j] = sum;
            
            /* Add some dependent operations */
            C[i][j] = complex_chain_1(C[i][j], A[i][j], B[i][j], (double)(i+j));
        }
    }
}

/* Test function mixing all types of operations */
static double comprehensive_test(int iterations) {
    double total = 0.0;
    
    /* Small matrix for memory operations */
    const int n = 8;
    double A[n][n], B[n][n], C[n][n];
    
    /* Initialize matrices */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = (double)(i * n + j) / 100.0;
            B[i][j] = (double)((i + j) * (i - j)) / 100.0;
        }
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Wide basic block test */
        total += test_scheduler_wide_block();
        
        /* Matrix operations */
        matrix_multiply(n, A, B, C);
        
        /* Use results to prevent dead code elimination */
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                total += C[i][j];
            }
        }
        
        /* Update matrices with complex operations */
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A[i][j] = complex_chain_1(A[i][j], B[i][j], C[i][j], total);
                B[i][j] = complex_chain_1(B[i][j], C[i][j], A[i][j], total + 1.0);
            }
        }
    }
    
    return total;
}

int main(void) {
    printf("Starting scheduler coverage test...\n");
    
    /* Seed RNG for variability */
    srand(time(NULL));
    
    /* Run comprehensive test */
    double result = comprehensive_test(3);
    
    /* Use result to prevent optimization */
    printf("Test result: %f\n", result);
    
    /* Additional targeted tests */
    printf("Running targeted scheduling tests...\n");
    
    /* Test 1: Wide basic blocks */
    double wide_result = test_scheduler_wide_block();
    printf("Wide block test: %f\n", wide_result);
    
    /* Test 2: Memory intensive */
    int arr1[64], arr2[64], arr3[64];
    for (int i = 0; i < 64; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
    }
    memory_ops(arr1, arr2, arr3, 64);
    printf("Memory test checksum: %d\n", arr3[0] + arr3[63]);
    
    /* Test 3: Speculative operations */
    int data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int spec_result = speculative_ops(rand() % 100, rand() % 100, data, 8);
    printf("Speculative test: %d\n", spec_result);
    
    /* Test 4: Control flow intensive */
    double cf_result = control_flow_intensive(rand() % 5, 
                                             (double)rand() / RAND_MAX,
                                             (double)rand() / RAND_MAX);
    printf("Control flow test: %f\n", cf_result);
    
    printf("All tests completed.\n");
    
    return 0;
}
