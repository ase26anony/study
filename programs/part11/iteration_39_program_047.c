/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler by creating complex basic blocks that require:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving for speculative scheduling
 * 3. Large instruction queues and ready lists
 * 4. Complex instruction mixes and dependencies
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

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile float g_volatile_float = 0.0f;
volatile double g_volatile_double = 0.0;

/* ======================================================================
 * Function 1: Complex arithmetic with mixed operations and dependencies
 * Creates long dependency chains and parallel computation paths
 * ====================================================================== */
static inline ALWAYS_INLINE 
int complex_arithmetic_block(int a, int b, int c, int d, int e, int f) {
    /* Long dependency chain */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 / (e + 1);
    int t5 = t4 ^ f;
    int t6 = t5 << 2;
    int t7 = t6 >> 1;
    int t8 = t7 & 0xFF;
    int t9 = t8 | 0x80;
    int t10 = t9 * t1;
    
    /* Parallel independent chains */
    int p1 = a * b;
    int p2 = c * d;
    int p3 = e * f;
    int p4 = p1 + p2;
    int p5 = p3 - p4;
    int p6 = p5 ^ p1;
    
    /* Mixed floating point operations */
    float ft1 = (float)t10 * 1.5f;
    float ft2 = ft1 + (float)p6;
    double dt1 = (double)ft2 * 2.71828;
    
    /* Memory operations with potential aliasing */
    int arr[8];
    arr[0] = t10;
    arr[1] = p6;
    arr[2] = (int)ft1;
    arr[3] = (int)dt1;
    
    /* More arithmetic to increase block size */
    for (int i = 4; i < 8; i++) {
        arr[i] = arr[i-1] * arr[i-2] + arr[i-3] - arr[i-4];
    }
    
    /* Final computation with all values */
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result += arr[i] * (i + 1);
    }
    
    return result + (int)dt1;
}

/* ======================================================================
 * Function 2: Loop with software pipelining opportunities
 * Small iteration count to encourage software pipelining attempts
 * ====================================================================== */
static inline ALWAYS_INLINE
float software_pipelined_loop(float init, int iterations) {
    float a = init;
    float b = init * 0.5f;
    float c = init * 0.25f;
    float d = init * 0.125f;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < iterations; i++) {
        /* Dependent operations creating pipeline stages */
        float t1 = a * 1.1f + (float)i;
        float t2 = b * 1.2f - t1;
        float t3 = c * 1.3f + t2;
        float t4 = d * 1.4f - t3;
        
        /* Update state for next iteration */
        a = t1 + t4 * 0.1f;
        b = t2 + a * 0.2f;
        c = t3 + b * 0.3f;
        d = t4 + c * 0.4f;
        
        /* Conditional that might cause speculative scheduling */
        if (d > 1000.0f) {
            d *= 0.9f;
        }
    }
    
    return a + b + c + d;
}

/* ======================================================================
 * Function 3: SIMD/Vector operations using GCC vector extensions
 * Creates many parallel operations that expand during scheduling
 * ====================================================================== */
static inline ALWAYS_INLINE
v4si vector_operations(v4si a, v4si b, v4si c, v4si d) {
    /* Multiple vector operations creating scheduling pressure */
    v4si v1 = a + b;
    v4si v2 = c * d;
    v4si v3 = v1 - v2;
    v4si v4 = v3 << 1;
    v4si v5 = v4 >> 2;
    v4si v6 = v5 & b;
    v4si v7 = v6 | a;
    v4si v8 = v7 ^ c;
    v4si v9 = v8 * d;
    v4si v10 = v9 + v1;
    
    /* Mixed with scalar operations */
    int s1 = v10[0] + v10[1];
    int s2 = v10[2] * v10[3];
    int s3 = s1 - s2;
    int s4 = s3 << 3;
    
    /* More vector operations */
    v4si v11 = v10 + s4;
    v4si v12 = v11 * 2;
    v4si v13 = v12 - v10;
    
    return v13;
}

/* ======================================================================
 * Function 4: Wide basic block with unrolled operations
 * Creates 50+ instructions in a single basic block
 * ====================================================================== */
static inline ALWAYS_INLINE
double wide_basic_block(double a, double b, double c) {
    /* Unrolled sequence of dependent operations */
    double r = a;
    
    /* 16 iterations * 4 operations each = 64 operations */
    #pragma GCC unroll 16
    for (int i = 0; i < 16; i++) {
        r = r * b + c;
        r = r / b - c;
        r = r + b * c;
        r = r - c / b;
    }
    
    /* Additional independent computation paths */
    double p1 = a * b;
    double p2 = a / b;
    double p3 = b * c;
    double p4 = b / c;
    double p5 = c * a;
    double p6 = c / a;
    
    /* Mix them together */
    double sum = p1 + p2 + p3 + p4 + p5 + p6;
    
    /* Trigonometric operations using different execution units */
    double t1 = sin(r);
    double t2 = cos(sum);
    double t3 = tan(r * 0.5);
    
    /* More arithmetic */
    double result = (t1 + t2) * t3;
    result = result / (1.0 + fabs(t1 - t2));
    
    /* Conditional that creates control flow complexity */
    if (result > 100.0) {
        result = log(fabs(result) + 1.0);
    } else if (result < -100.0) {
        result = exp(-result);
    } else {
        result = sqrt(fabs(result));
    }
    
    return result;
}

/* ======================================================================
 * Function 5: Function with inline assembly creating scheduling barriers
 * Also uses memory operations with aliasing concerns
 * ====================================================================== */
static inline ALWAYS_INLINE
int mixed_assembly_and_c(int x, int y, int* arr, int size) {
    int result = x;
    
    /* Memory operations that might alias */
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * x + y;
    }
    
    /* Inline assembly creating scheduling barrier */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (result), "r" (y)
        : "%eax"
    );
    
    /* More C operations */
    for (int i = 0; i < size; i++) {
        result += arr[i];
        arr[i] = result ^ arr[i];
    }
    
    /* Another assembly block */
    int temp;
    asm volatile (
        "cpuid\n\t"
        : "=a" (temp)
        : "a" (0)
        : "%ebx", "%ecx", "%edx"
    );
    
    /* Final computations */
    result = result * 2 - temp;
    
    return result;
}

/* ======================================================================
 * Function 6: Complex control flow with switch statement
 * Forces frontend state saving for speculative scheduling
 * ====================================================================== */
static inline ALWAYS_INLINE
int switch_with_computation(int x, int mode) {
    int result = x;
    
    /* Pre-switch computations */
    int a = x * 2;
    int b = x + 100;
    int c = x / 3;
    int d = x - 50;
    
    /* Switch with multiple cases - creates complex control flow */
    switch (mode % 8) {
        case 0:
            result = a + b * c - d;
            /* Additional computations in case */
            result = result ^ 0xAAAAAAAA;
            result = result << 3;
            break;
        case 1:
            result = b - c + d * a;
            result = result | 0x55555555;
            result = result >> 2;
            break;
        case 2:
            result = c * d + a - b;
            result = result & 0x0F0F0F0F;
            result = result * 3;
            break;
        case 3:
            result = d / (a + 1) + b - c;
            result = result ^ 0xFFFFFFFF;
            result = result + 1;
            break;
        case 4:
            result = (a + b) * (c - d);
            result = result % 1000;
            result = result * 7;
            break;
        case 5:
            result = (b - c) / (d + 1) * a;
            result = result & 0xFF;
            result = result | 0x80;
            break;
        case 6:
            result = (c * d) + (a / b);
            result = result << 4;
            result = result >> 1;
            break;
        case 7:
            result = (d - a) * (b + c);
            result = result ^ result;
            result = result + 100;
            break;
    }
    
    /* Post-switch computations */
    result = result * 2;
    result = result + (a ^ b ^ c ^ d);
    
    return result;
}

/* ======================================================================
 * Main test driver that calls all functions with varying parameters
 * Ensures computations aren't optimized away
 * ====================================================================== */
int main() {
    int total_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Seed for reproducible results */
    srand(42);
    
    /* Test 1: Complex arithmetic block */
    printf("Test 1: Complex arithmetic...\n");
    for (int i = 0; i < 100; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100 + 1;
        int d = rand() % 100;
        int e = rand() % 100;
        int f = rand() % 100;
        
        int r = complex_arithmetic_block(a, b, c, d, e, f);
        total_result += r;
        g_volatile_int = r; /* Prevent optimization */
    }
    
    /* Test 2: Software pipelined loops */
    printf("Test 2: Software pipelining...\n");
    for (int i = 0; i < 50; i++) {
        float init = (float)(rand() % 1000) / 10.0f;
        int iterations = 4 + (rand() % 4); /* Small iteration counts */
        
        float r = software_pipelined_loop(init, iterations);
        float_result += r;
        g_volatile_float = r;
    }
    
    /* Test 3: Vector operations */
    printf("Test 3: Vector operations...\n");
    v4si va = {1, 2, 3, 4};
    v4si vb = {5, 6, 7, 8};
    v4si vc = {9, 10, 11, 12};
    v4si vd = {13, 14, 15, 16};
    
    for (int i = 0; i < 50; i++) {
        v4si vr = vector_operations(va, vb, vc, vd);
        total_result += vr[0] + vr[1] + vr[2] + vr[3];
        
        /* Modify vectors for next iteration */
        va[0] += 1;
        vb[1] += 1;
        vc[2] += 1;
        vd[3] += 1;
    }
    
    /* Test 4: Wide basic blocks */
    printf("Test 4: Wide basic blocks...\n");
    for (int i = 0; i < 30; i++) {
        double a = (double)(rand() % 1000) / 100.0;
        double b = (double)(rand() % 1000) / 100.0 + 0.1;
        double c = (double)(rand() % 1000) / 100.0;
        
        double r = wide_basic_block(a, b, c);
        double_result += r;
        g_volatile_double = r;
    }
    
    /* Test 5: Mixed assembly and C */
    printf("Test 5: Mixed assembly and C...\n");
    int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = rand() % 100;
    }
    
    for (int i = 0; i < 40; i++) {
        int x = rand() % 100;
        int y = rand() % 100;
        
        int r = mixed_assembly_and_c(x, y, arr, 16);
        total_result += r;
        
        /* Modify array to create different memory patterns */
        arr[i % 16] = r;
    }
    
    /* Test 6: Switch with complex control flow */
    printf("Test 6: Complex control flow...\n");
    for (int i = 0; i < 100; i++) {
        int x = rand() % 1000;
        int mode = rand();
        
        int r = switch_with_computation(x, mode);
        total_result += r;
    }
    
    /* Combine all results to prevent dead code elimination */
    total_result += (int)float_result;
    total_result += (int)double_result;
    
    printf("Final checksum: %d\n", total_result);
    printf("Float component: %f\n", float_result);
    printf("Double component: %f\n", double_result);
    
    /* Use results to affect return value */
    return (total_result % 256) == 0 ? 0 : 1;
}
