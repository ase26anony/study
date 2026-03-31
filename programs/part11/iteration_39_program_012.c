/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the specific cleanup logic in GCC's
 * Haifa scheduler (free_sched_block function in haifa-sched.cc).
 * It creates complex basic blocks that require extensive instruction
 * scheduling with target hooks, frontend state saving, and large instruction
 * queues.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline)) inline

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimizations */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ============================================
 * Test Function 1: Mixed Integer and FP operations
 * with dependency chains and parallel paths
 * ============================================ */
ALWAYS_INLINE static int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Multiple dependent operations */
    int t1 = a * b + c;
    int t2 = t1 / (d + 1);
    int t3 = t2 << (e & 3);
    int t4 = t3 ^ (t1 * t2);
    int t5 = t4 - (t2 >> 2);
    int t6 = t5 * 7 + t3;
    int t7 = t6 & 0xFFF;
    int t8 = t7 | (t4 << 16);
    return t8;
}

ALWAYS_INLINE static float complex_float_chain(float a, float b, float c, float d) {
    /* FP dependency chain */
    float t1 = a * b + c;
    float t2 = sinf(t1) * d;
    float t3 = cosf(t2) + t1;
    float t4 = t3 * t2 - a;
    float t5 = sqrtf(fabsf(t4)) + b;
    float t6 = t5 * 0.5f + t3;
    return t6;
}

/* Function with speculative scheduling opportunities */
static int test_mixed_operations(int iterations) {
    int result = 0;
    float fp_acc = 0.0f;
    double dp_acc = 0.0;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Integer dependency chain */
        int int_val = complex_int_chain(i, i+1, i+2, i+3, i+4);
        
        /* Floating-point dependency chain */
        float fp_val = complex_float_chain(i*0.1f, (i+1)*0.2f, (i+2)*0.3f, (i+3)*0.4f);
        
        /* Double precision operations */
        double dp_val = sin(i * 0.05) * cos(i * 0.03);
        
        /* Memory operations with potential aliasing */
        volatile int* mem_ptr = &global_seed;
        *mem_ptr += int_val;
        
        /* Conditional that creates scheduling barriers */
        if (int_val & 1) {
            fp_acc += fp_val;
            dp_acc += dp_val * 2.0;
        } else {
            fp_acc -= fp_val * 0.5f;
            dp_acc -= dp_val;
        }
        
        /* More integer operations */
        result += (int_val ^ (int)(fp_val * 100)) * (i & 0xF);
        
        /* Function call with side effects */
        global_float += fp_val * 0.01f;
        global_double += dp_val * 0.005;
    }
    
    /* Final mixed computation */
    result += (int)(fp_acc * 10) + (int)(dp_acc * 5);
    return result;
}

/* ============================================
 * Test Function 2: Wide basic block with 
 * unrolled loops and vector operations
 * ============================================ */
static int test_vector_wide_block(int size) {
    /* Create wide basic block through unrolling */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Manual unrolling to create wide block */
    for (int i = 0; i < size; i += 8) {
        /* Independent computation paths */
        int a1 = i * 3 + 1;
        int a2 = (i+1) * 5 - 2;
        int a3 = (i+2) * 7 + 3;
        int a4 = (i+3) * 11 - 4;
        int a5 = (i+4) * 13 + 5;
        int a6 = (i+5) * 17 - 6;
        int a7 = (i+6) * 19 + 7;
        int a8 = (i+7) * 23 - 8;
        
        /* More operations on each path */
        a1 = (a1 << 3) | (a1 >> 5);
        a2 = (a2 * 3) ^ (a2 >> 2);
        a3 = (a3 + 0xABCD) & 0xFFFF;
        a4 = (a4 - 0x1234) | 0xFF;
        a5 = (a5 * 7) % 1023;
        a6 = (a6 ^ 0xDEAD) + 0xBEEF;
        a7 = (a7 << 1) + (a7 >> 1);
        a8 = (a8 * 9) - (a8 / 3);
        
        /* Floating point operations mixed in */
        float f1 = sinf(i * 0.1f);
        float f2 = cosf((i+1) * 0.2f);
        float f3 = sqrtf(fabsf(i * 0.3f));
        float f4 = logf(1.0f + fabsf(i * 0.4f));
        
        /* Vector operations using GCC vector extensions */
        v4si v1 = {a1, a2, a3, a4};
        v4si v2 = {a5, a6, a7, a8};
        v4si v3 = v1 + v2;
        v4si v4 = v1 * v2;
        v4si v5 = v3 & v4;
        
        /* Extract results from vectors */
        int vsum = v5[0] + v5[1] + v5[2] + v5[3];
        
        /* More FP operations */
        fsum += f1 * f2 + f3 - f4;
        
        /* Final accumulation with conditional */
        sum += (vsum > 0) ? vsum : -vsum;
        sum += (int)(fsum * 100);
    }
    
    return sum;
}

/* ============================================
 * Test Function 3: Complex control flow
 * with switches and computed gotos
 * ============================================ */
static int test_complex_control_flow(int value) {
    int result = value;
    
    /* Switch creates complex basic block boundaries */
    switch (value % 8) {
        case 0: {
            /* Block with many operations */
            for (int i = 0; i < 4; i++) {
                result = (result * 3 + 1) & 0x7FFF;
                float ftmp = sinf(result * 0.01f);
                result += (int)(ftmp * 1000);
            }
            break;
        }
        case 1: {
            /* Different operation mix */
            result ^= 0x12345678;
            result = (result << 16) | (result >> 16);
            double dtmp = cos(result * 0.001);
            result += (int)(dtmp * 10000);
            break;
        }
        case 2: {
            /* Memory intensive */
            int array[16];
            for (int i = 0; i < 16; i++) {
                array[i] = result + i;
                result ^= array[i];
            }
            break;
        }
        case 3: {
            /* FP intensive */
            float f = result * 0.1f;
            for (int i = 0; i < 6; i++) {
                f = sinf(f) * 2.0f;
                f = cosf(f) / 1.5f;
            }
            result += (int)(f * 100);
            break;
        }
        default: {
            /* Mixed operations */
            result = result * 7 - 3;
            result = (result & 0xAA) | ((result & 0x55) << 1);
            volatile int* ptr = &global_seed;
            *ptr += result;
            break;
        }
    }
    
    /* Additional loop with small iteration count for software pipelining */
    for (int i = 0; i < 5; i++) {
        result = (result + i) * (result - i);
        if (result & 1) {
            result ^= 0xFFFFFFFF;
        }
    }
    
    return result;
}

/* ============================================
 * Test Function 4: Matrix operations
 * creating many independent computation paths
 * ============================================ */
static int test_matrix_operations(int dim) {
    /* Small matrix multiplication with unrolling */
    int A[4][4], B[4][4], C[4][4];
    int sum = 0;
    
    /* Initialize matrices */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            A[i][j] = (i * 4 + j + 1) * dim;
            B[i][j] = (j * 4 + i + 1) * (dim + 1);
            C[i][j] = 0;
        }
    }
    
    /* Manual unrolled multiplication for wide basic block */
    for (int i = 0; i < 4; i++) {
        /* Unroll j loop */
        C[i][0] = A[i][0]*B[0][0] + A[i][1]*B[1][0] + A[i][2]*B[2][0] + A[i][3]*B[3][0];
        C[i][1] = A[i][0]*B[0][1] + A[i][1]*B[1][1] + A[i][2]*B[2][1] + A[i][3]*B[3][1];
        C[i][2] = A[i][0]*B[0][2] + A[i][1]*B[1][2] + A[i][2]*B[2][2] + A[i][3]*B[3][2];
        C[i][3] = A[i][0]*B[0][3] + A[i][1]*B[1][3] + A[i][2]*B[2][3] + A[i][3]*B[3][3];
        
        /* Additional operations on results */
        C[i][0] = (C[i][0] << 2) | (C[i][0] >> 6);
        C[i][1] = (C[i][1] * 3) ^ 0xAA;
        C[i][2] = C[i][2] + (C[i][2] >> 1);
        C[i][3] = C[i][3] - (C[i][3] & 0xF);
        
        /* Accumulate with floating point conversion */
        float fval = sqrtf(fabsf(C[i][0] * 0.01f));
        sum += C[i][0] + C[i][1] + C[i][2] + C[i][3] + (int)(fval * 10);
    }
    
    return sum;
}

/* ============================================
 * Test Function 5: Inline assembly to force
 * target-specific scheduling hooks
 * ============================================ */
static int test_with_assembly(int x) {
    int result = x;
    
    /* Mix of C and assembly operations */
    for (int i = 0; i < 8; i++) {
        /* C operations */
        result = result * 3 + 1;
        
        /* Inline assembly - creates scheduling barriers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull $0x1234, %%eax\n\t"
            "addl $0x5678, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (result)
            : "r" (result)
            : "%eax"
        );
        
        /* More C operations */
        float f = result * 0.001f;
        f = sinf(f) * 2.0f;
        result += (int)(f * 1000);
        
        /* Another assembly block */
        asm volatile (
            "rorl $8, %0\n\t"
            "xorl $0xF0F0F0F0, %0\n\t"
            : "+r" (result)
            :
            : "cc"
        );
    }
    
    return result;
}

/* ============================================
 * Main driver function
 * ============================================ */
int main(int argc, char** argv) {
    int total_result = 0;
    
    printf("Starting scheduler coverage test...\n");
    
    /* Seed for reproducibility */
    srand(global_seed);
    
    /* Test 1: Mixed operations with dependency chains */
    printf("Running test_mixed_operations...\n");
    int r1 = test_mixed_operations(100);
    printf("  Result 1: %d\n", r1);
    total_result ^= r1;
    
    /* Test 2: Wide basic blocks with vector operations */
    printf("Running test_vector_wide_block...\n");
    int r2 = test_vector_wide_block(64);
    printf("  Result 2: %d\n", r2);
    total_result ^= r2;
    
    /* Test 3: Complex control flow */
    printf("Running test_complex_control_flow...\n");
    int r3 = test_complex_control_flow(12345);
    printf("  Result 3: %d\n", r3);
    total_result ^= r3;
    
    /* Test 4: Matrix operations */
    printf("Running test_matrix_operations...\n");
    int r4 = test_matrix_operations(10);
    printf("  Result 4: %d\n", r4);
    total_result ^= r4;
    
    /* Test 5: Assembly operations */
    printf("Running test_with_assembly...\n");
    int r5 = test_with_assembly(999);
    printf("  Result 5: %d\n", r5);
    total_result ^= r5;
    
    /* Additional combined test for maximum coverage */
    printf("Running combined stress test...\n");
    for (int i = 0; i < 10; i++) {
        int tmp = test_mixed_operations(20 + i);
        tmp += test_vector_wide_block(16 + i);
        tmp ^= test_complex_control_flow(tmp);
        total_result += tmp;
    }
    
    printf("Final checksum: %d\n", total_result);
    printf("Global state: seed=%d, float=%.3f, double=%.3f\n", 
           global_seed, global_float, global_double);
    
    return total_result != 0 ? 0 : 1;
}
