/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that require:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex instruction mixes with dependencies
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

/* ============================================
 * Function 1: Complex arithmetic with mixed types
 * Creates dependency chains and uses different execution units
 * ============================================ */
ALWAYS_INLINE static float complex_arithmetic_chain(int iterations) {
    float a = 1.0f, b = 2.0f, c = 3.0f, d = 4.0f;
    int i1 = 5, i2 = 6, i3 = 7, i4 = 8;
    double dp1 = 1.5, dp2 = 2.5;
    
    /* Mixed integer and floating-point dependency chain */
    for (int i = 0; i < iterations; i++) {
        /* Integer chain */
        i1 = i2 * i3 + i4;
        i2 = i1 - i3 * i4;
        i3 = i2 / (i4 + 1);
        i4 = i1 ^ i2 | i3;
        
        /* Floating-point chain with dependencies */
        a = b * c + d;
        b = c - d / a;
        c = d + a * b;
        d = a / (b + 1.0f) - c;
        
        /* Double precision chain */
        dp1 = dp2 * 1.1 + dp1;
        dp2 = dp1 / 0.9 - dp2;
        
        /* Memory operations with potential aliasing */
        global_float_result += a + b + c + d;
        global_double_result += dp1 + dp2;
    }
    
    return a + b + c + d + (float)(dp1 + dp2) + i1 + i2 + i3 + i4;
}

/* ============================================
 * Function 2: Vector operations with unrolled loops
 * Creates wide basic blocks with many independent operations
 * ============================================ */
ALWAYS_INLINE static v4sf vector_operations(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Unrolled vector operations - creates many parallel instructions */
    v4sf r1 = a + b * c;
    v4sf r2 = a - b / c;
    v4sf r3 = a * b + c;
    v4sf r4 = a / b - c;
    v4sf r5 = r1 + r2 * r3;
    v4sf r6 = r1 - r2 / r3;
    v4sf r7 = r4 * r5 + r6;
    v4sf r8 = r4 / r5 - r6;
    v4sf r9 = r7 + r8 * r1;
    v4sf r10 = r7 - r8 / r1;
    
    /* More independent chains */
    v4sf t1 = b + c * d;
    v4sf t2 = b - c / d;
    v4sf t3 = c * d + a;
    v4sf t4 = c / d - a;
    v4sf t5 = t1 + t2 * t3;
    v4sf t6 = t1 - t2 / t3;
    v4sf t7 = t4 * t5 + t6;
    v4sf t8 = t4 / t5 - t6;
    
    /* Cross dependencies to force scheduling complexity */
    return r10 + t8 + (r9 * t7) - (r8 / t6) + (r7 + t5) * (r6 - t4);
}

/* ============================================
 * Function 3: Memory-intensive computation with aliasing
 * Creates scheduling barriers and requires state tracking
 * ============================================ */
ALWAYS_INLINE static double memory_intensive_computation(double* arr1, double* arr2, 
                                                         int size, int iterations) {
    double sum = 0.0;
    
    /* Small unrolled loop - may trigger software pipelining */
    for (int iter = 0; iter < iterations; iter++) {
        /* Multiple independent memory operations */
        double t1 = arr1[0] * arr2[0];
        double t2 = arr1[1] / arr2[1];
        double t3 = arr1[2] + arr2[2];
        double t4 = arr1[3] - arr2[3];
        
        /* Dependent computations */
        double r1 = t1 + t2 * t3;
        double r2 = t1 - t2 / t3;
        double r3 = t4 * r1 + r2;
        double r4 = t4 / r1 - r2;
        
        /* More operations with potential aliasing */
        arr1[0] = r1;
        arr2[0] = r2;
        arr1[1] = r3;
        arr2[1] = r4;
        
        /* Conditional update - may trigger speculative scheduling */
        if (r1 > r2) {
            sum += r1 * r3 - r2 / r4;
        } else {
            sum += r2 * r4 - r1 / r3;
        }
        
        /* Function call with side effects (inline asm) */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* ============================================
 * Function 4: Complex control flow with switches
 * Triggers frontend state saving requirements
 * ============================================ */
ALWAYS_INLINE static int control_flow_intensive(int x, int y, int z) {
    int result = 0;
    
    /* Multiple conditional branches in a basic block */
    if (x > y) {
        result = x * y + z;
        if (z > 0) {
            result += x / y;
            asm volatile("" : "+r"(result) : : "memory");
        } else {
            result -= y / (x + 1);
        }
    } else if (x < y) {
        result = y * z - x;
        if (z < 0) {
            result *= 2;
            asm volatile("" : "+r"(result) : : "memory");
        }
    } else {
        result = x + y + z;
    }
    
    /* Switch statement - creates complex control flow */
    switch (result % 8) {
        case 0: result ^= x; break;
        case 1: result |= y; break;
        case 2: result &= z; break;
        case 3: result <<= (x & 3); break;
        case 4: result >>= (y & 3); break;
        case 5: result = ~result; break;
        case 6: result = result * 3 + 1; break;
        case 7: result = result / 2 - 1; break;
    }
    
    /* More arithmetic to increase block size */
    result = result * 3 + x * 5 - y * 7 + z * 11;
    result = (result & 0xFF) | ((result >> 8) & 0xFF) << 8;
    
    return result;
}

/* ============================================
 * Function 5: Mixed SIMD and scalar operations
 * Exercises target-specific scheduling hooks
 * ============================================ */
#ifdef __SSE4_2__
#include <nmmintrin.h>
ALWAYS_INLINE static __m128 sse_operations(__m128 a, __m128 b, __m128 c) {
    /* Mix of SSE4.2 instructions */
    __m128 r1 = _mm_add_ps(a, b);
    __m128 r2 = _mm_mul_ps(b, c);
    __m128 r3 = _mm_sub_ps(c, a);
    __m128 r4 = _mm_div_ps(a, b);
    
    __m128 r5 = _mm_dp_ps(r1, r2, 0xFF);  /* SSE4.2 dot product */
    __m128 r6 = _mm_max_ps(r3, r4);
    __m128 r7 = _mm_min_ps(r5, r6);
    __m128 r8 = _mm_round_ps(r7, _MM_FROUND_TO_NEAREST_INT);
    
    /* Scalar operations mixed with SIMD */
    float f1 = ((float*)&r8)[0];
    float f2 = ((float*)&r8)[1];
    float f3 = ((float*)&r8)[2];
    float f4 = ((float*)&r8)[3];
    
    float sf1 = f1 * f2 + f3 - f4;
    float sf2 = f1 / f2 * f3 + f4;
    float sf3 = f1 + f2 * f3 / f4;
    float sf4 = f1 - f2 + f3 * f4;
    
    __m128 sr = _mm_set_ps(sf4, sf3, sf2, sf1);
    
    return _mm_add_ps(r8, sr);
}
#endif

/* ============================================
 * Main test driver with multiple complex functions
 * Each function call creates a basic block that needs scheduling
 * ============================================ */
int main() {
    clock_t start = clock();
    double total_result = 0.0;
    
    /* Initialize arrays for memory operations */
    double arr1[8], arr2[8];
    for (int i = 0; i < 8; i++) {
        arr1[i] = (double)(i + 1) * 1.1;
        arr2[i] = (double)(i + 1) * 0.9;
    }
    
    /* Initialize vectors for SIMD operations */
    v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Test 1: Complex arithmetic chains */
    printf("Test 1: Complex arithmetic chains...\n");
    for (int i = 0; i < 100; i++) {
        float r = complex_arithmetic_chain(4);  /* Small iteration count may trigger SW pipelining */
        total_result += r;
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Test 2: Vector operations */
    printf("Test 2: Vector operations...\n");
    for (int i = 0; i < 50; i++) {
        v4sf vr = vector_operations(va, vb, vc, vd);
        total_result += ((float*)&vr)[0] + ((float*)&vr)[1] + 
                       ((float*)&vr)[2] + ((float*)&vr)[3];
        
        /* Modify vectors to create different patterns */
        va[0] += 0.1f; vb[1] -= 0.1f; vc[2] *= 1.01f; vd[3] /= 1.01f;
    }
    
    /* Test 3: Memory-intensive computation */
    printf("Test 3: Memory-intensive computation...\n");
    for (int i = 0; i < 20; i++) {
        double mr = memory_intensive_computation(arr1, arr2, 8, 8);
        total_result += mr;
        
        /* Shuffle arrays */
        double temp = arr1[0];
        arr1[0] = arr1[7];
        arr1[7] = temp;
    }
    
    /* Test 4: Control flow intensive */
    printf("Test 4: Control flow intensive...\n");
    for (int i = 0; i < 1000; i++) {
        int cr = control_flow_intensive(i, i*2, i*3);
        total_result += cr;
    }
    
#ifdef __SSE4_2__
    /* Test 5: SSE operations if available */
    printf("Test 5: SSE operations...\n");
    __m128 sa = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sb = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 sc = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    
    for (int i = 0; i < 100; i++) {
        __m128 sr = sse_operations(sa, sb, sc);
        total_result += ((float*)&sr)[0] + ((float*)&sr)[1] + 
                       ((float*)&sr)[2] + ((float*)&sr)[3];
        
        sa = _mm_add_ps(sa, _mm_set1_ps(0.01f));
    }
#endif
    
    /* Test 6: Combined large basic block */
    printf("Test 6: Combined large basic block...\n");
    {
        /* Create a very wide basic block with many independent operations */
        double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0, d5 = 5.0;
        float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f, f4 = 4.0f, f5 = 5.0f;
        int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
        
        /* 50+ independent and dependent operations in one basic block */
        d1 = d2 * d3 + d4 / d5 - d1;
        f1 = f2 * f3 + f4 / f5 - f1;
        i1 = i2 * i3 + i4 / i5 - i1;
        
        d2 = d3 * d4 + d5 / d1 - d2;
        f2 = f3 * f4 + f5 / f1 - f2;
        i2 = i3 * i4 + i5 / i1 - i2;
        
        d3 = d4 * d5 + d1 / d2 - d3;
        f3 = f4 * f5 + f1 / f2 - f3;
        i3 = i4 * i5 + i1 / i2 - i3;
        
        d4 = d5 * d1 + d2 / d3 - d4;
        f4 = f5 * f1 + f2 / f3 - f4;
        i4 = i5 * i1 + i2 / i3 - i4;
        
        d5 = d1 * d2 + d3 / d4 - d5;
        f5 = f1 * f2 + f3 / f4 - f5;
        i5 = i1 * i2 + i3 / i4 - i5;
        
        /* More operations... */
        for (int j = 0; j < 4; j++) {
            d1 = sin(d1) + cos(d2);
            d2 = exp(d3) * log(fabs(d4) + 1.0);
            d3 = pow(d5, 1.5) + sqrt(d1);
            d4 = atan(d2) * acos(d3 / 10.0);
            d5 = tanh(d4) + erf(d5);
            
            f1 = f1 * 1.1f + f2 * 0.9f;
            f2 = f2 * 1.2f - f3 * 0.8f;
            f3 = f3 * 1.3f + f4 * 0.7f;
            f4 = f4 * 1.4f - f5 * 0.6f;
            f5 = f5 * 1.5f + f1 * 0.5f;
            
            i1 = (i1 << 3) | (i2 >> 2);
            i2 = (i2 << 2) ^ (i3 >> 3);
            i3 = (i3 << 1) & (i4 >> 1);
            i4 = (i4 << 4) | (i5 >> 4);
            i5 = (i5 << 2) ^ (i1 >> 2);
        }
        
        total_result += d1 + d2 + d3 + d4 + d5 + f1 + f2 + f3 + f4 + f5 + i1 + i2 + i3 + i4 + i5;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %f\n", total_result);
    printf("Global float result: %f\n", (float)global_float_result);
    printf("Global double result: %f\n", global_double_result);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("All tests completed.\n");
    
    return (total_result > 0) ? 0 : 1;
}
