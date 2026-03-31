/* test_sched_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for SIMD-like operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_accumulator = 0.0f;

/* Function with side effects to create scheduling barriers */
static ALWAYS_INLINE int side_effect_func(int x) {
    /* Inline assembly creates a scheduling barrier */
    asm volatile("" : "+r" (x) : : "memory");
    return x * 2 + 1;
}

/* Complex integer arithmetic with dependency chains */
static ALWAYS_INLINE void test_integer_deps(int *arr, int n) {
    int a = arr[0];
    int b = arr[1];
    int c = arr[2];
    int d = arr[3];
    int e = arr[4];
    
    /* Long dependency chain */
    a = b + c;          /* 1 */
    d = a * e;          /* 2 - depends on 1 */
    b = d - c;          /* 3 - depends on 2 */
    e = b / 2;          /* 4 - depends on 3 */
    a = e << 3;         /* 5 - depends on 4 */
    c = a | 0xFF;       /* 6 - depends on 5 */
    d = c ^ b;          /* 7 - depends on 6 and 3 */
    e = d % 17;         /* 8 - depends on 7 */
    
    /* Parallel independent chains */
    int x = arr[5] * 2;
    int y = arr[6] + 3;
    int z = arr[7] - 1;
    int w = arr[8] / 4;
    
    /* Mix with side effects */
    x = side_effect_func(x);
    y = side_effect_func(y);
    z = side_effect_func(z);
    w = side_effect_func(w);
    
    /* More dependencies */
    arr[0] = a + x;
    arr[1] = b + y;
    arr[2] = c + z;
    arr[3] = d + w;
    arr[4] = e + x + y;
}

/* Mixed integer and floating-point operations */
static ALWAYS_INLINE void test_mixed_ops(float *farr, int *iarr, int n) {
    float fa = farr[0];
    float fb = farr[1];
    float fc = farr[2];
    int ia = iarr[0];
    int ib = iarr[1];
    int ic = iarr[2];
    
    /* Interleaved FP and integer ops */
    fa = fb * fc;               /* FP mul */
    ia = ib + ic;               /* Integer add */
    fb = sinf(fa) + 1.0f;       /* FP function */
    ib = ia << 2;               /* Integer shift */
    fc = fa * 2.5f;             /* FP mul */
    ic = ib * 3;                /* Integer mul */
    
    /* More complex mixing */
    for (int i = 0; i < 4; i++) {
        fa = fa * fb + (float)ia;
        ia = (int)fa + ib;
        fb = fb / fc - (float)ic;
        ib = ia ^ ic;
        fc = sqrtf(fabsf(fc)) + 1.0f;
        ic = ib % 256;
    }
    
    farr[0] = fa;
    farr[1] = fb;
    farr[2] = fc;
    iarr[0] = ia;
    iarr[1] = ib;
    iarr[2] = ic;
}

/* Function with vector operations (triggers SIMD scheduling) */
static ALWAYS_INLINE void test_vector_ops(v4si *vec_int, v4sf *vec_float, int n) {
    v4si va = vec_int[0];
    v4si vb = vec_int[1];
    v4si vc = vec_int[2];
    
    v4sf vfa = vec_float[0];
    v4sf vfb = vec_float[1];
    v4sf vfc = vec_float[2];
    
    /* Vector integer operations */
    va = vb + vc;
    vb = va * vc;
    vc = vb - va;
    va = vc << 2;
    vb = va | vc;
    vc = vb ^ va;
    
    /* Vector float operations */
    vfa = vfb * vfc;
    vfb = vfa + vfc;
    vfc = vfb / vfa;
    vfa = vfc * 2.5f;
    
    /* Mixed vector operations */
    for (int i = 0; i < 3; i++) {
        va[i] = (int)(vfa[i] * 10.0f);
        vfa[i] = (float)(vb[i] + vc[i]);
    }
    
    vec_int[0] = va;
    vec_int[1] = vb;
    vec_int[2] = vc;
    vec_float[0] = vfa;
    vec_float[1] = vfb;
    vec_float[2] = vfc;
}

/* Wide basic block from unrolled loop */
static ALWAYS_INLINE void test_wide_block(int *arr, float *farr, int n) {
    /* Unrolled loop creates many instructions */
    #pragma GCC unroll 8
    for (int i = 0; i < n && i < 32; i++) {
        /* Multiple independent chains per iteration */
        int a = arr[i];
        int b = arr[(i + 1) % n];
        float fa = farr[i];
        float fb = farr[(i + 1) % n];
        
        /* Chain 1 */
        a = a * 3 + b;
        b = b - a / 2;
        a = a ^ (b << 1);
        
        /* Chain 2 */
        fa = fa * 1.5f + fb;
        fb = sinf(fa) * 0.5f;
        fa = fa / (fabsf(fb) + 1.0f);
        
        /* Chain 3 - mixed */
        int c = (int)(fa * 100.0f);
        float fc = (float)(a + b) / 10.0f;
        
        /* Chain 4 - with side effect */
        c = side_effect_func(c);
        fc = fc * 2.0f + (float)c;
        
        arr[i] = a + c;
        farr[i] = fa + fc;
    }
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int test_speculative(int *arr, int n) {
    int result = 0;
    
    /* Complex conditional with many operations */
    if (arr[0] > 0) {
        /* Block with many dependent operations */
        int a = arr[0];
        int b = arr[1];
        int c = arr[2];
        
        a = b * c + a;
        b = a / (c + 1);
        c = (a << 3) | (b >> 2);
        a = c ^ b;
        b = a * 3 - c;
        c = b % 17 + a;
        
        result = a + b + c;
    } else {
        /* Alternative block with different ops */
        float fa = (float)arr[0];
        float fb = (float)arr[1];
        float fc = (float)arr[2];
        
        fa = fb * fc - fa;
        fb = fa / (fc + 1.0f);
        fc = sqrtf(fabsf(fa)) + fb;
        fa = fc * 2.5f - fb;
        fb = sinf(fa) * cosf(fc);
        fc = fa + fb * 3.0f;
        
        result = (int)(fa + fb + fc);
    }
    
    /* Switch statement for more control flow complexity */
    switch (result % 4) {
        case 0:
            result = result * 2 + 1;
            break;
        case 1:
            result = result / 2 - 3;
            break;
        case 2:
            result = (result << 1) | 0xF;
            break;
        case 3:
            result = result ^ 0xAAAA;
            break;
    }
    
    return result;
}

/* Main test function combining all patterns */
static void run_scheduling_tests(int iterations) {
    int int_data[64];
    float float_data[64];
    v4si vec_int_data[8];
    v4sf vec_float_data[8];
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        int_data[i] = (i * 3 + 1) % 256;
        float_data[i] = (float)i * 0.1f;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            vec_int_data[i][j] = (i * 4 + j) * 2;
            vec_float_data[i][j] = (float)(i * 4 + j) * 0.25f;
        }
    }
    
    int checksum = 0;
    float fchecksum = 0.0f;
    
    /* Run multiple iterations to ensure scheduling happens */
    for (int iter = 0; iter < iterations; iter++) {
        /* Test 1: Integer dependency chains */
        test_integer_deps(int_data, 16);
        
        /* Test 2: Mixed operations */
        test_mixed_ops(float_data, int_data, 16);
        
        /* Test 3: Vector operations (triggers SIMD scheduling) */
        test_vector_ops(vec_int_data, vec_float_data, 4);
        
        /* Test 4: Wide basic block */
        test_wide_block(int_data, float_data, 32);
        
        /* Test 5: Speculative scheduling */
        int_data[0] = (iter % 2 == 0) ? 1 : -1;
        int spec_result = test_speculative(int_data, 16);
        
        /* Update checksums */
        for (int i = 0; i < 16; i++) {
            checksum += int_data[i];
            fchecksum += float_data[i];
        }
        checksum += spec_result;
        
        /* Modify data for next iteration */
        for (int i = 0; i < 64; i++) {
            int_data[i] = (int_data[i] * 1103515245 + 12345) & 0x7FFFFFFF;
            float_data[i] = fmodf(float_data[i] * 1.1f, 100.0f);
        }
    }
    
    /* Use results to prevent optimization */
    global_accumulator += fchecksum;
    printf("Checksum: %d, Float checksum: %f\n", checksum, fchecksum);
}

/* Additional test with matrix operations */
static void test_matrix_scheduling(int size) {
    float mat_a[16][16];
    float mat_b[16][16];
    float mat_c[16][16];
    
    /* Initialize matrices */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            mat_a[i][j] = (float)(i * size + j) * 0.01f;
            mat_b[i][j] = (float)((i + 1) * (j + 1)) * 0.02f;
            mat_c[i][j] = 0.0f;
        }
    }
    
    /* Matrix multiplication with manual unrolling */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float sum = 0.0f;
            
            /* Unrolled inner loop */
            #pragma GCC unroll 4
            for (int k = 0; k < size; k++) {
                /* Complex expression with multiple ops */
                float a_val = mat_a[i][k];
                float b_val = mat_b[k][j];
                
                /* Dependency chain within loop body */
                a_val = a_val * 1.5f;
                b_val = b_val / 2.0f;
                float prod = a_val * b_val;
                prod = prod + (a_val + b_val) * 0.5f;
                prod = sinf(prod * 0.1f) * 10.0f;
                
                sum += prod;
                
                /* Additional independent operation */
                mat_a[i][k] = fmodf(mat_a[i][k] * 1.01f, 10.0f);
            }
            
            mat_c[i][j] = sum;
            
            /* Conditional update */
            if (mat_c[i][j] > 5.0f) {
                mat_c[i][j] = sqrtf(mat_c[i][j]);
            } else {
                mat_c[i][j] = mat_c[i][j] * mat_c[i][j];
            }
        }
    }
    
    /* Compute final checksum */
    float total = 0.0f;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            total += mat_c[i][j];
        }
    }
    
    global_accumulator += total;
    printf("Matrix total: %f\n", total);
}

/* Main driver */
int main(int argc, char **argv) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    printf("Running scheduling coverage tests...\n");
    
    /* Run primary tests */
    run_scheduling_tests(iterations);
    
    /* Run matrix test */
    test_matrix_scheduling(8);
    
    /* Additional test with different data patterns */
    for (int pattern = 0; pattern < 3; pattern++) {
        run_scheduling_tests(10);
    }
    
    printf("All tests completed. Global accumulator: %f\n", global_accumulator);
    
    return 0;
}
