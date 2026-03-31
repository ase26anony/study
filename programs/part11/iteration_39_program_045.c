/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Prevent excessive inlining to keep basic blocks large */
#define NOINLINE __attribute__((noinline))

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ============================================
 * Function 1: Mixed Integer and FP Operations
 * Creates dependency chains and independent paths
 * ============================================ */
NOINLINE static int complex_mixed_operations(int n, float* restrict farr, int* restrict iarr) {
    int i;
    int sum_int = 0;
    float sum_float = 0.0f;
    
    /* Wide basic block with mixed operations */
    for (i = 0; i < n; i++) {
        /* Integer dependency chain */
        int a = iarr[i] + global_seed;
        int b = a * 3;
        int c = b - iarr[i];
        int d = c / 2;
        int e = d ^ 0x55AA55AA;
        
        /* Floating-point dependency chain */
        float fa = farr[i] * global_float;
        float fb = fa + (float)i;
        float fc = fb / 2.0f;
        float fd = fc - farr[i];
        float fe = fd * 3.14159f;
        
        /* Independent parallel computations */
        int p1 = iarr[i] * 7;
        int p2 = iarr[i] + 11;
        int p3 = p1 - p2;
        
        float fp1 = farr[i] * 2.71828f;
        float fp2 = farr[i] + 1.61803f;
        float fp3 = fp1 / fp2;
        
        /* More mixed operations with dependencies */
        int g = e + (int)fe;
        float fg = fe + (float)g;
        
        /* Conditional update creating scheduling barriers */
        if (g > 1000) {
            sum_int += g * 2;
            sum_float += fg * 1.5f;
        } else {
            sum_int += p3;
            sum_float += fp3;
        }
        
        /* Memory operations with potential aliasing */
        iarr[i] = g;
        farr[i] = fg;
    }
    
    return sum_int + (int)sum_float;
}

/* ============================================
 * Function 2: Vector Operations with SIMD-like patterns
 * Triggers target-specific scheduling hooks
 * ============================================ */
NOINLINE static void vector_operations(v4si* restrict vi, v4sf* restrict vf, int count) {
    int i;
    v4si vseed = {global_seed, global_seed + 1, global_seed + 2, global_seed + 3};
    v4sf vpi = {3.14159f, 3.14159f, 3.14159f, 3.14159f};
    
    /* Unrolled loop creates wide basic block */
    for (i = 0; i < count; i += 2) {
        /* Vector integer operations */
        v4si va = vi[i] + vseed;
        v4si vb = va * (v4si){2, 3, 4, 5};
        v4si vc = vb - vi[i];
        v4si vd = vc / (v4si){2, 2, 2, 2};
        
        /* Vector float operations */
        v4sf vfa = vf[i] * vpi;
        v4sf vfb = vfa + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vfc = vfb / (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
        
        /* Mixed vector operations */
        v4si ve = vd + (v4si)vfc;
        v4sf vfd = vfc + (v4sf)ve;
        
        /* Store results */
        vi[i] = ve;
        vf[i] = vfd;
        
        /* Second iteration in unrolled loop */
        if (i + 1 < count) {
            v4si va2 = vi[i + 1] + vseed;
            v4si vb2 = va2 * (v4si){3, 4, 5, 6};
            v4si vc2 = vb2 - vi[i + 1];
            v4si vd2 = vc2 / (v4si){3, 3, 3, 3};
            
            v4sf vfa2 = vf[i + 1] * vpi;
            v4sf vfb2 = vfa2 + (v4sf){5.0f, 6.0f, 7.0f, 8.0f};
            v4sf vfc2 = vfb2 / (v4sf){3.0f, 3.0f, 3.0f, 3.0f};
            
            v4si ve2 = vd2 + (v4si)vfc2;
            v4sf vfd2 = vfc2 + (v4sf)ve2;
            
            vi[i + 1] = ve2;
            vf[i + 1] = vfd2;
        }
    }
}

/* ============================================
 * Function 3: Complex Control Flow with Speculation
 * Forces frontend state saving (restore_state)
 * ============================================ */
NOINLINE static double speculative_scheduling(int iterations, double* data) {
    double result = 0.0;
    int i, j;
    
    /* Outer loop with inner speculative operations */
    for (i = 0; i < iterations; i++) {
        double acc = data[i];
        
        /* Inner loop with small iteration count (4-8) */
        for (j = 0; j < 6; j++) {
            /* Dependent FP operations */
            double t1 = acc * global_double;
            double t2 = t1 + (double)j;
            double t3 = t2 / 1.41421356;
            double t4 = t3 - acc;
            double t5 = t4 * 2.71828182;
            
            /* Conditional that may be speculatively scheduled */
            if (t5 > 100.0) {
                acc = t5 * 0.9;
            } else if (t5 < -100.0) {
                acc = t5 * 1.1;
            } else {
                acc = t5 + (double)(i * j);
            }
            
            /* More operations in the basic block */
            double s1 = sin(acc);
            double s2 = cos(acc);
            double s3 = s1 * s2;
            acc = s3 * 0.5;
        }
        
        /* Switch-like control flow */
        int mod = i % 4;
        switch (mod) {
            case 0:
                result += acc * 1.0;
                break;
            case 1:
                result += acc * 1.5;
                break;
            case 2:
                result += acc * 2.0;
                break;
            case 3:
                result += acc * 2.5;
                /* Fall through to create more scheduling complexity */
            default:
                result += 1.0;
                break;
        }
        
        /* Memory operation with barrier */
        data[i] = acc;
    }
    
    return result;
}

/* ============================================
 * Function 4: Large Basic Block with Many Independent Paths
 * Fills instruction queues and ready lists
 * ============================================ */
NOINLINE static long wide_basic_block(int* arr, int size) {
    long total = 0;
    int i;
    
    /* Manual loop unrolling creates very wide basic block */
    for (i = 0; i < size; i += 8) {
        int idx0 = i;
        int idx1 = i + 1 < size ? i + 1 : i;
        int idx2 = i + 2 < size ? i + 2 : i;
        int idx3 = i + 3 < size ? i + 3 : i;
        int idx4 = i + 4 < size ? i + 4 : i;
        int idx5 = i + 5 < size ? i + 5 : i;
        int idx6 = i + 6 < size ? i + 6 : i;
        int idx7 = i + 7 < size ? i + 7 : i;
        
        /* 8 independent computation paths */
        int val0 = arr[idx0] * 3 + 7;
        int val1 = arr[idx1] * 5 - 11;
        int val2 = arr[idx2] * 7 + 13;
        int val3 = arr[idx3] * 11 - 17;
        int val4 = arr[idx4] * 13 + 19;
        int val5 = arr[idx5] * 17 - 23;
        int val6 = arr[idx6] * 19 + 29;
        int val7 = arr[idx7] * 23 - 31;
        
        /* More operations on each path */
        val0 = (val0 << 3) | (val0 >> 29);
        val1 = (val1 << 5) | (val1 >> 27);
        val2 = (val2 << 7) | (val2 >> 25);
        val3 = (val3 << 9) | (val3 >> 23);
        val4 = (val4 << 11) | (val4 >> 21);
        val5 = (val5 << 13) | (val5 >> 19);
        val6 = (val6 << 15) | (val6 >> 17);
        val7 = (val7 << 17) | (val7 >> 15);
        
        /* Dependency chains within each path */
        int chain0 = val0 * 2;
        chain0 = chain0 + val0;
        chain0 = chain0 / 3;
        
        int chain1 = val1 * 3;
        chain1 = chain1 + val1;
        chain1 = chain1 / 4;
        
        int chain2 = val2 * 4;
        chain2 = chain2 + val2;
        chain2 = chain2 / 5;
        
        int chain3 = val3 * 5;
        chain3 = chain3 + val3;
        chain3 = chain3 / 6;
        
        int chain4 = val4 * 6;
        chain4 = chain4 + val4;
        chain4 = chain4 / 7;
        
        int chain5 = val5 * 7;
        chain5 = chain5 + val5;
        chain5 = chain5 / 8;
        
        int chain6 = val6 * 8;
        chain6 = chain6 + val6;
        chain6 = chain6 / 9;
        
        int chain7 = val7 * 9;
        chain7 = chain7 + val7;
        chain7 = chain7 / 10;
        
        /* Conditional updates */
        if (chain0 > 0) total += chain0;
        if (chain1 > 0) total += chain1;
        if (chain2 > 0) total += chain2;
        if (chain3 > 0) total += chain3;
        if (chain4 > 0) total += chain4;
        if (chain5 > 0) total += chain5;
        if (chain6 > 0) total += chain6;
        if (chain7 > 0) total += chain7;
        
        /* Store results */
        arr[idx0] = chain0;
        arr[idx1] = chain1;
        arr[idx2] = chain2;
        arr[idx3] = chain3;
        arr[idx4] = chain4;
        arr[idx5] = chain5;
        arr[idx6] = chain6;
        arr[idx7] = chain7;
    }
    
    return total;
}

/* ============================================
 * Function 5: Inline Assembly Creating Scheduling Barriers
 * Forces scheduler to work around non-movable instructions
 * ============================================ */
NOINLINE static int asm_barriers(int x, int y) {
    int result;
    
    /* Complex sequence with inline assembly */
    int a = x * y;
    int b = x + y;
    
    /* Assembly barrier that scheduler must respect */
    asm volatile ("# Assembly Barrier 1" : : : "memory");
    
    int c = a - b;
    int d = c * 2;
    
    /* Another barrier */
    asm volatile ("# Assembly Barrier 2\n\t"
                  "nop" : : : "memory");
    
    int e = d / 3;
    int f = e ^ 0x12345678;
    
    /* Dependent operations after barriers */
    for (int i = 0; i < 4; i++) {
        f = f * 3 + i;
        if (f > 1000) {
            f = f / 2;
        } else {
            f = f * 2;
        }
    }
    
    asm volatile ("# Final Assembly Barrier" : : : "memory");
    
    result = f;
    return result;
}

/* ============================================
 * Main Driver Function
 * Calls all test functions with different parameters
 * ============================================ */
int main(void) {
    const int SIZE = 256;
    const int VECTOR_SIZE = 64;
    
    /* Allocate and initialize test data */
    float* farr = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int* iarr = (int*)aligned_alloc(16, SIZE * sizeof(int));
    double* darr = (double*)aligned_alloc(16, SIZE * sizeof(double));
    v4si* vi = (v4si*)aligned_alloc(16, VECTOR_SIZE * sizeof(v4si));
    v4sf* vf = (v4sf*)aligned_alloc(16, VECTOR_SIZE * sizeof(v4sf));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        farr[i] = (float)i * 1.5f;
        iarr[i] = i * 3;
        darr[i] = (double)i * 0.75;
    }
    
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vi[i] = (v4si){i, i+1, i+2, i+3};
        vf[i] = (v4sf){(float)i, (float)(i+1), (float)(i+2), (float)(i+3)};
    }
    
    long total = 0;
    
    /* Call each test function multiple times with different parameters */
    printf("Starting scheduler coverage test...\n");
    
    /* Test 1: Mixed operations */
    for (int i = 0; i < 10; i++) {
        total += complex_mixed_operations(SIZE / (i + 1), farr, iarr);
    }
    printf("Test 1 complete: %ld\n", total);
    
    /* Test 2: Vector operations */
    vector_operations(vi, vf, VECTOR_SIZE);
    printf("Test 2 complete\n");
    
    /* Test 3: Speculative scheduling */
    double spec_result = 0.0;
    for (int i = 0; i < 5; i++) {
        spec_result += speculative_scheduling(50, darr);
    }
    printf("Test 3 complete: %f\n", spec_result);
    
    /* Test 4: Wide basic blocks */
    long wide_result = 0;
    for (int i = 0; i < 100; i++) {
        wide_result += wide_basic_block(iarr, SIZE);
    }
    printf("Test 4 complete: %ld\n", wide_result);
    
    /* Test 5: Assembly barriers */
    int asm_result = 0;
    for (int i = 0; i < 1000; i++) {
        asm_result += asm_barriers(i, i * 2);
    }
    printf("Test 5 complete: %d\n", asm_result);
    
    /* Final checksum to prevent dead code elimination */
    long final_checksum = total + (long)spec_result + wide_result + asm_result;
    
    /* Use results to prevent optimization */
    for (int i = 0; i < 10; i++) {
        final_checksum += iarr[i] + (int)farr[i] + (int)darr[i];
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    
    /* Cleanup */
    free(farr);
    free(iarr);
    free(darr);
    free(vi);
    free(vf);
    
    return (final_checksum > 0) ? 0 : 1;
}
