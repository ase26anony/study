/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the specific cleanup logic in GCC's
 * Haifa scheduler (free_sched_block function in haifa-sched.cc).
 * It creates complex basic blocks that force the scheduler to allocate
 * and use the full scheduling context, including target-specific hooks,
 * frontend state saving, and large instruction queues.
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

/* ======================================================================
 * Function 1: Complex dependency chains with mixed operations
 * This creates long dependency chains that require careful scheduling
 * and may trigger frontend state saving due to speculative scheduling.
 * ====================================================================== */
static ALWAYS_INLINE double complex_dependency_chain(int iterations, double seed) {
    double a = seed * 1.1;
    double b = seed * 0.9;
    double c = seed * 1.5;
    double result = 0.0;
    
    /* Complex dependency chain with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain - forces scheduler to track many dependencies */
        a = sin(a) * cos(b) + tan(c);
        b = cos(a) * sin(b) - tan(a);
        c = tan(b) * cos(c) + sin(a);
        
        /* Integer operations mixed in */
        int ia = (int)(a * 1000);
        int ib = (int)(b * 1000);
        int ic = (int)(c * 1000);
        
        /* More dependencies */
        a = (ia % 100) * 0.01 + (ib & 0xFF) * 0.001 + (ic | 0x7F) * 0.0001;
        b = (ib % 200) * 0.02 + (ic & 0x7F) * 0.002 + (ia | 0xFF) * 0.0002;
        c = (ic % 300) * 0.03 + (ia & 0x3F) * 0.003 + (ib | 0x3F) * 0.0003;
        
        /* Conditional that may cause speculative scheduling */
        if (a > b && b > c) {
            result += a - b + c;
        } else if (a < b && b < c) {
            result += b - a - c;
        } else {
            result += a + b + c;
        }
        
        /* Memory operations with potential aliasing */
        double* ptrs[3] = {&a, &b, &c};
        for (int j = 0; j < 3; j++) {
            *ptrs[j] += j * 0.1;
        }
    }
    
    return result;
}

/* ======================================================================
 * Function 2: Wide basic block with many independent operations
 * This creates a large instruction queue and ready list by unrolling
 * loops and having many parallel computation paths.
 * ====================================================================== */
static ALWAYS_INLINE v4sf wide_basic_block_vector(v4sf vec_a, v4sf vec_b, v4sf vec_c) {
    /* Many independent vector operations - will fill the ready list */
    v4sf r1 = vec_a + vec_b;
    v4sf r2 = vec_a - vec_b;
    v4sf r3 = vec_a * vec_b;
    v4sf r4 = vec_a / (vec_b + 1.0f);
    
    v4sf r5 = vec_b + vec_c;
    v4sf r6 = vec_b - vec_c;
    v4sf r7 = vec_b * vec_c;
    v4sf r8 = vec_b / (vec_c + 1.0f);
    
    v4sf r9 = vec_c + vec_a;
    v4sf r10 = vec_c - vec_a;
    v4sf r11 = vec_c * vec_a;
    v4sf r12 = vec_c / (vec_a + 1.0f);
    
    /* Mix operations to create some dependencies */
    v4sf r13 = r1 * r5 + r9;
    v4sf r14 = r2 * r6 + r10;
    v4sf r15 = r3 * r7 + r11;
    v4sf r16 = r4 * r8 + r12;
    
    /* More independent chains */
    v4sf r17 = r13 * 0.5f;
    v4sf r18 = r14 * 0.6f;
    v4sf r19 = r15 * 0.7f;
    v4sf r20 = r16 * 0.8f;
    
    v4sf r21 = r17 + r18;
    v4sf r22 = r19 + r20;
    v4sf r23 = r21 - r22;
    v4sf r24 = r21 * r22;
    
    /* Final result mixing all computations */
    return r23 + r24 + r13 + r14 + r15 + r16;
}

/* ======================================================================
 * Function 3: Matrix operations with unrolled loops
 * Creates wide basic blocks through loop unrolling, forcing large
 * instruction queues during scheduling.
 * ====================================================================== */
static ALWAYS_INLINE void matrix_multiply_4x4(float A[4][4], float B[4][4], float C[4][4]) {
    /* Manually unrolled 4x4 matrix multiplication */
    /* Row 0 */
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0] + A[0][3]*B[3][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1] + A[0][2]*B[2][1] + A[0][3]*B[3][1];
    C[0][2] = A[0][0]*B[0][2] + A[0][1]*B[1][2] + A[0][2]*B[2][2] + A[0][3]*B[3][2];
    C[0][3] = A[0][0]*B[0][3] + A[0][1]*B[1][3] + A[0][2]*B[2][3] + A[0][3]*B[3][3];
    
    /* Row 1 */
    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0] + A[1][2]*B[2][0] + A[1][3]*B[3][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1] + A[1][2]*B[2][1] + A[1][3]*B[3][1];
    C[1][2] = A[1][0]*B[0][2] + A[1][1]*B[1][2] + A[1][2]*B[2][2] + A[1][3]*B[3][2];
    C[1][3] = A[1][0]*B[0][3] + A[1][1]*B[1][3] + A[1][2]*B[2][3] + A[1][3]*B[3][3];
    
    /* Row 2 */
    C[2][0] = A[2][0]*B[0][0] + A[2][1]*B[1][0] + A[2][2]*B[2][0] + A[2][3]*B[3][0];
    C[2][1] = A[2][0]*B[0][1] + A[2][1]*B[1][1] + A[2][2]*B[2][1] + A[2][3]*B[3][1];
    C[2][2] = A[2][0]*B[0][2] + A[2][1]*B[1][2] + A[2][2]*B[2][2] + A[2][3]*B[3][2];
    C[2][3] = A[2][0]*B[0][3] + A[2][1]*B[1][3] + A[2][2]*B[2][3] + A[2][3]*B[3][3];
    
    /* Row 3 */
    C[3][0] = A[3][0]*B[0][0] + A[3][1]*B[1][0] + A[3][2]*B[2][0] + A[3][3]*B[3][0];
    C[3][1] = A[3][0]*B[0][1] + A[3][1]*B[1][1] + A[3][2]*B[2][1] + A[3][3]*B[3][1];
    C[3][2] = A[3][0]*B[0][2] + A[3][1]*B[1][2] + A[3][2]*B[2][2] + A[3][3]*B[3][2];
    C[3][3] = A[3][0]*B[0][3] + A[3][1]*B[1][3] + A[3][2]*B[2][3] + A[3][3]*B[3][3];
}

/* ======================================================================
 * Function 4: Mixed integer/FP with function calls
 * Includes inline assembly and function calls to create scheduling
 * barriers and potentially trigger target-specific scheduling hooks.
 * ====================================================================== */
static ALWAYS_INLINE long mixed_operations_with_barriers(int n, float* arr) {
    long sum_int = 0;
    double sum_fp = 0.0;
    
    /* Mixed operations with potential for software pipelining */
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int val1 = i * 3;
        int val2 = i / 2;
        int val3 = i % 7;
        
        /* Floating point operations */
        float fval1 = arr[i] * 1.5f;
        float fval2 = arr[i] / 2.0f;
        double dval = (double)fval1 * (double)fval2;
        
        /* Dependency chain */
        val1 = val1 + val2 * val3;
        val2 = val1 ^ (val3 << 2);
        val3 = (val1 & 0xFF) | (val2 & 0xFF00);
        
        fval1 = fval1 + sinf(fval2);
        fval2 = cosf(fval1) * tanf(dval);
        dval = fval1 * fval2 + dval;
        
        /* Inline assembly - creates scheduling barrier */
        asm volatile("" : "+r"(val1), "+r"(val2) : : "memory");
        
        /* Function call with side effects - another scheduling barrier */
        arr[i] = fval1 + fval2;
        
        /* Conditional with multiple branches - may trigger state saving */
        switch (i % 4) {
            case 0:
                sum_int += val1;
                sum_fp += dval;
                break;
            case 1:
                sum_int += val2;
                sum_fp -= dval;
                break;
            case 2:
                sum_int += val3;
                sum_fp *= 1.01;
                break;
            case 3:
                sum_int -= val1;
                sum_fp /= 1.01;
                break;
        }
        
        /* Memory operations with aliasing */
        float* ptr1 = &arr[i];
        float* ptr2 = &arr[(i + 1) % n];
        *ptr1 = *ptr1 * 0.99f;
        *ptr2 = *ptr2 * 1.01f;
    }
    
    return sum_int + (long)sum_fp;
}

/* ======================================================================
 * Function 5: SIMD operations for x86/ARM targets
 * Uses GCC vector extensions to create SIMD operations that may
 * trigger target-specific scheduling hooks.
 * ====================================================================== */
static ALWAYS_INLINE v4si simd_operations(v4si a, v4si b, v4si c) {
    /* Multiple SIMD operations - target specific scheduling */
    v4si r1 = a + b;
    v4si r2 = a - b;
    v4si r3 = a * b;
    v4si r4 = (a << 2) | (b >> 1);
    
    v4si r5 = b + c;
    v4si r6 = b - c;
    v4si r7 = b * c;
    v4si r8 = (b << 1) | (c >> 2);
    
    v4si r9 = c + a;
    v4si r10 = c - a;
    v4si r11 = c * a;
    v4si r12 = (c << 3) | (a >> 3);
    
    /* Mix them together */
    v4si r13 = r1 + r5 + r9;
    v4si r14 = r2 + r6 + r10;
    v4si r15 = r3 + r7 + r11;
    v4si r16 = r4 + r8 + r12;
    
    /* More operations to increase scheduling pressure */
    v4si r17 = r13 * r14;
    v4si r18 = r15 * r16;
    v4si r19 = r17 & r18;
    v4si r20 = r17 | r18;
    v4si r21 = r19 ^ r20;
    v4si r22 = ~r21;
    
    return r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20 + r21 + r22;
}

/* ======================================================================
 * Main test driver
 * Calls all test functions with different parameters to ensure
 * the scheduler processes various types of basic blocks.
 * ====================================================================== */
int main() {
    clock_t start = clock();
    double total_result = 0.0;
    
    printf("Starting scheduler coverage test...\n");
    
    /* Test 1: Complex dependency chains */
    printf("Test 1: Complex dependency chains...\n");
    for (int i = 0; i < 100; i++) {
        total_result += complex_dependency_chain(8, i * 0.1);
    }
    
    /* Test 2: Wide basic blocks with vector operations */
    printf("Test 2: Wide basic blocks with vector operations...\n");
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_c = {0.1f, 0.2f, 0.3f, 0.4f};
    
    v4sf vec_result = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 50; i++) {
        vec_result += wide_basic_block_vector(vec_a, vec_b, vec_c);
        vec_a += 0.1f;
        vec_b += 0.2f;
        vec_c += 0.3f;
    }
    
    /* Test 3: Matrix operations (unrolled loops) */
    printf("Test 3: Matrix operations...\n");
    float A[4][4], B[4][4], C[4][4];
    for (int iter = 0; iter < 20; iter++) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                A[i][j] = (i * 4 + j) * 0.1f;
                B[i][j] = (j * 4 + i) * 0.2f;
            }
        }
        matrix_multiply_4x4(A, B, C);
        
        /* Use result to prevent optimization */
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                total_result += C[i][j];
            }
        }
    }
    
    /* Test 4: Mixed operations with barriers */
    printf("Test 4: Mixed operations with barriers...\n");
    float arr[64];
    for (int i = 0; i < 64; i++) {
        arr[i] = i * 0.01f;
    }
    
    for (int i = 0; i < 10; i++) {
        long barrier_result = mixed_operations_with_barriers(64, arr);
        total_result += barrier_result * 0.001;
    }
    
    /* Test 5: SIMD operations */
    printf("Test 5: SIMD operations...\n");
    v4si simd_a = {1, 2, 3, 4};
    v4si simd_b = {5, 6, 7, 8};
    v4si simd_c = {9, 10, 11, 12};
    
    v4si simd_result = {0, 0, 0, 0};
    for (int i = 0; i < 40; i++) {
        simd_result += simd_operations(simd_a, simd_b, simd_c);
        simd_a += 1;
        simd_b += 2;
        simd_c += 3;
    }
    
    /* Add SIMD result to total */
    int* simd_ptr = (int*)&simd_result;
    for (int i = 0; i < 4; i++) {
        total_result += simd_ptr[i];
    }
    
    /* Final computation to use all results */
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %f\n", total_result);
    printf("Elapsed time: %f seconds\n", elapsed);
    printf("Scheduler coverage test completed.\n");
    
    /* Return value based on result to prevent dead code elimination */
    return (fabs(total_result) > 1000.0) ? 0 : 1;
}
